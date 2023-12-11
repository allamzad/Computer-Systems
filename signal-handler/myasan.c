#define _GNU_SOURCE 1

#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "util.h"

#define PAGE_SIZE 4096
typedef uint8_t page_t[PAGE_SIZE];

static void *const START_PAGE = (void *) ((size_t) 1 << 32);
static const size_t MAX_HEAP_SIZE = (size_t) 1 << 30;
static const int HEAP_MMAP_FLAGS = MAP_ANONYMOUS | MAP_PRIVATE;
static const size_t HEADER_MAGIC = 0x0123456789ABCDEF;

typedef struct {
    size_t magic;
    size_t size;
    bool is_allocated;
} header_t;

static bool is_initialized = false;
static page_t *current_page;

static void sigsegv_handler(int signum, siginfo_t *siginfo, void *context) {
    (void) signum;
    (void) context;

    void *addr = (void *) siginfo->si_addr;

    if (!is_initialized || current_page == NULL) {
        report_seg_fault(addr);
        return;
    }
    if (addr >= START_PAGE && addr < (void *) (current_page + 1)) {
        mprotect(START_PAGE, MAX_HEAP_SIZE, PROT_NONE);
        report_invalid_heap_access(addr);
        return;
    }
    report_seg_fault(addr);
    return;
}

static size_t pages_round_up(size_t size) {
    return (size + PAGE_SIZE - 1) / PAGE_SIZE;
}

static void set_header(page_t *header_page, size_t size, bool is_allocated) {
    mprotect((void *) header_page, sizeof(page_t), PROT_READ | PROT_WRITE);
    memcpy((void *) header_page, &HEADER_MAGIC, sizeof(HEADER_MAGIC));
    memcpy((void *) header_page + sizeof(HEADER_MAGIC), &size, sizeof(size));
    memcpy((void *) header_page + sizeof(HEADER_MAGIC) + sizeof(size), &is_allocated,
           sizeof(is_allocated));
    mprotect((void *) header_page, sizeof(page_t), PROT_NONE);
}

static void *get_payload(page_t *header_page, size_t size) {
    return (void *) header_page + PAGE_SIZE + pages_round_up(size) * PAGE_SIZE - size;
}

static void check_for_leaks(void) {
    // Prevent memory leaks from stdout
    fclose(stdout);

    // TODO: Edit this in stage 4
    void *current_addr = START_PAGE;
    void *header = (void *) current_addr;

    while ((page_t *) header < current_page) {
        header = (void *) current_addr;
        mprotect((void *) header, sizeof(page_t), PROT_READ | PROT_WRITE);
        size_t size = *(size_t *) ((void *) header + sizeof(size_t));
        bool allocated = *(bool *) ((void *) header + 2 * sizeof(size_t));
        if (allocated) {
            mprotect(START_PAGE, MAX_HEAP_SIZE, PROT_NONE);
            report_memory_leak(get_payload((page_t *) header, size), size);
            return;
        }
        current_addr = (void *) header + PAGE_SIZE + pages_round_up(size) * PAGE_SIZE;
        mprotect((void *) header, sizeof(page_t), PROT_NONE);
    }
    return;
}

static void asan_init(void) {
    if (is_initialized) {
        return;
    }

    // Avoid buffering on stdout
    setbuf(stdout, NULL);

    current_page = mmap(START_PAGE, MAX_HEAP_SIZE,
                        PROT_NONE, // TODO: Edit this in stage 5
                        HEAP_MMAP_FLAGS, -1, 0);
    assert(current_page == START_PAGE);

    atexit(check_for_leaks);

    is_initialized = true;

    struct sigaction act = {.sa_sigaction = sigsegv_handler, .sa_flags = SA_SIGINFO};
    sigaction(SIGSEGV, &act, NULL);
}

void *malloc(size_t size) {
    asan_init();

    size_t pages_necessary = pages_round_up(size);
    mprotect((void *) current_page + PAGE_SIZE, sizeof(page_t) * pages_necessary,
             PROT_READ | PROT_WRITE);

    // Store the size of the allocation at the beginning of the page before the payload
    mprotect((void *) current_page, sizeof(page_t), PROT_READ | PROT_WRITE);
    page_t *header_page = current_page;
    set_header(header_page, size, true);
    mprotect((void *) current_page, sizeof(page_t), PROT_NONE);
    current_page += 1 + pages_necessary;

    // Provide the user with the END of the first page
    return get_payload(header_page, size);
}

void free(void *ptr) {
    asan_init();

    if (ptr == NULL) {
        return;
    }
    if (ptr < START_PAGE + PAGE_SIZE || ptr > START_PAGE + MAX_HEAP_SIZE * PAGE_SIZE) {
        mprotect(START_PAGE, MAX_HEAP_SIZE, PROT_NONE);
        report_invalid_free(ptr);
        return;
    }

    // TODO: Edit this in stages 2 & 3
    void *header =
        (void *) ptr - PAGE_SIZE - ((void *) ptr - (void *) START_PAGE) % PAGE_SIZE;
    mprotect((void *) header, sizeof(page_t), PROT_READ | PROT_WRITE);
    size_t size = *(size_t *) ((void *) header + sizeof(size_t));

    if (*(size_t *) header != HEADER_MAGIC ||
        get_payload((page_t *) header, size) != ptr) {
        mprotect(START_PAGE, MAX_HEAP_SIZE, PROT_NONE);
        report_invalid_free(ptr);
        return;
    }

    bool allocated = *(bool *) ((void *) header + 2 * sizeof(size_t));
    if (allocated == false) {
        mprotect(START_PAGE, MAX_HEAP_SIZE, PROT_NONE);
        report_double_free(ptr, size);
        return;
    }
    set_header((page_t *) header, size, false);
    mprotect((void *) header, sizeof(page_t), PROT_NONE);
    mprotect((void *) header + PAGE_SIZE, sizeof(page_t) * pages_round_up(size),
             PROT_NONE);
    return;
}
