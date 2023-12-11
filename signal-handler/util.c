#include "util.h"

#include <execinfo.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

// Status codes for ASAN errors
static const int SEG_FAULT = 10;
static const int INVALID_HEAP_ACCESS = 11;
static const int INVALID_FREE = 20;
static const int DOUBLE_FREE = 21;
static const int MEMORY_LEAK = 30;

static const size_t MAX_BACKTRACE_SIZE = 10;
static const char ASAN_SO[] = "bin/libmyasan.so";
// This may need to be updated alongside libc updates, otherwise backtrace filtering may break
static const char RESTORE_RT_LOCATION[] = "/lib/x86_64-linux-gnu/libc.so.6(+0x43090)";
static const char MAIN_ADDRESS[] = "main+";
static const char ADDR2LINE[] = "llvm-addr2line-12";
static const char ADDR2LINE_FLAGS[] = "-ifse";

__attribute__((format(printf, 1, 2))) static void asan_warn(const char *fmt, ...) {
    bool colorize = isatty(STDERR_FILENO);
    if (colorize) {
        fprintf(stderr, "\033[0;31m");
    }
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (colorize) {
        fprintf(stderr, "\033[0m");
    }
}

static void run_addr2line(const char *executable, const char *address) {
    int child = fork();
    if (child == 0) {
        // Make addr2line print to stderr instead of stdout
        int result = dup2(STDERR_FILENO, STDOUT_FILENO);
        assert(result >= 0);
        const char *argv[] = {ADDR2LINE, ADDR2LINE_FLAGS, executable, address, NULL};
        execvp(ADDR2LINE, (char **) argv);
        assert(false);
    }

    int result = wait(NULL);
    assert(result == child);
}

static void print_backtrace(void) {
    void *return_addresses[MAX_BACKTRACE_SIZE];
    size_t backtrace_size = backtrace(return_addresses, MAX_BACKTRACE_SIZE);
    char **backtrace_lines = backtrace_symbols(return_addresses, backtrace_size);
    assert(backtrace_lines != NULL);
    for (size_t i = 0; i < backtrace_size; i++) {
        // Skip all lines from inside ASAN
        char *backtrace_line = backtrace_lines[i];
        if (strncmp(backtrace_line, ASAN_SO, strlen(ASAN_SO)) == 0) {
            continue;
        }
        // And the signal handler return function
        char *location_end = strchr(backtrace_line, ' ');
        assert(location_end != NULL);
        *location_end = '\0';
        if (strcmp(backtrace_line, RESTORE_RT_LOCATION) == 0) {
            continue;
        }

        asan_warn("at %s\n", backtrace_line);
        char *filename_end = strchr(backtrace_line, '(');
        assert(filename_end != NULL);
        *filename_end = '\0';
        char *address;
        if (filename_end[1] == '+') {
            // Function from shared object
            address = &filename_end[2];
            char *address_end = strchr(address, ')');
            assert(address_end != NULL);
            *address_end = '\0';
        }
        else {
            // Function in executable
            assert(location_end[1] == '[');
            address = location_end + 2;
            char *address_end = strchr(address, ']');
            assert(address_end != NULL);
            *address_end = '\0';
        }

        run_addr2line(backtrace_line, address);
        // Stop after backtracing main()
        if (strncmp(&filename_end[1], MAIN_ADDRESS, strlen(MAIN_ADDRESS)) == 0) {
            break;
        }
    }
    free(backtrace_lines);
}

void report_seg_fault(void *address) {
    asan_warn("Segmentation fault: unknown address %p\n", address);
    print_backtrace();
    _exit(SEG_FAULT);
}

void report_invalid_heap_access(void *address) {
    asan_warn(
        "Invalid heap access: address %p is not in an allocation or was already freed\n",
        address);
    print_backtrace();
    _exit(INVALID_HEAP_ACCESS);
}

void report_invalid_free(void *address) {
    asan_warn("Invalid free(): %p is not an allocation\n", address);
    print_backtrace();
    _exit(INVALID_FREE);
}

void report_double_free(void *allocation, size_t allocation_size) {
    asan_warn("Double free(): allocation of %zu bytes at %p was already freed\n",
              allocation_size, allocation);
    print_backtrace();
    _exit(DOUBLE_FREE);
}

void report_memory_leak(void *allocation, size_t allocation_size) {
    asan_warn("Memory leak: allocation of %zu bytes at %p was never freed\n",
              allocation_size, allocation);
    // No backtrace is printed because the callee is not at fault
    _exit(MEMORY_LEAK);
}
