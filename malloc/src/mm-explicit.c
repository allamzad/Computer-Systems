/*
 * mm-explicit.c - The best malloc package EVAR!
 *
 * TODO (bug): Uh..this is an implicit list???
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memlib.h"
#include "mm.h"

/** The layout of each block allocated on the heap */
typedef struct {
    size_t header;
    uint8_t payload[];
} block_t;

typedef struct free_block free_block_t;

struct free_block {
    size_t header;
    free_block_t *next;
    free_block_t *prev;
};

/** The required alignment of heap payloads */
const size_t ALIGNMENT = 2 * sizeof(size_t);

/** The first and last free blocks on the heap */
static block_t *mm_heap_first = NULL;
static block_t *mm_heap_last = NULL;

static free_block_t *mm_free_first = NULL;
static free_block_t *mm_free_last = NULL;

/** Rounds up `size` to the nearest multiple of `n` */
static size_t round_up(size_t size, size_t n) {
    return (size + (n - 1)) / n * n;
}

/** Set's a block's header with the given size and allocation state */
static void set_header(block_t *block, size_t size, bool is_allocated) {
    block->header = size | is_allocated;
}

/** Set's a block's footer with the given size and allocation state */
static void set_footer(block_t *block, size_t size, bool is_allocated) {
    size_t *footer = (size_t *) (((void *) block) + size - sizeof(size_t));
    *footer = size | is_allocated;
}

/** Extracts a block's size from its header */
static size_t get_size(block_t *block) {
    return block->header & ~1;
}

/** Extracts a block's allocation state from its header */
static bool is_allocated(block_t *block) {
    return block->header & 1;
}

static void add_to_freelist(block_t *block) {
    free_block_t *freed_block = (free_block_t *) block;
    size_t freed_block_size = get_size((block_t *) freed_block);

    set_header((block_t *) freed_block, freed_block_size, false);
    set_footer((block_t *) freed_block, freed_block_size, false);

    if (mm_free_first == NULL) {
        mm_free_first = freed_block;
        mm_free_first->prev = NULL;
        mm_free_first->next = NULL;
        mm_free_last = freed_block;
    }
    else {
        mm_free_first->prev = freed_block;
        freed_block->next = mm_free_first;
        mm_free_first = freed_block;
    }
}

static free_block_t *remove_from_freelist(free_block_t *curr) {
    // Curr is the only block
    if (curr == mm_free_first && curr == mm_free_last) {
        mm_free_first = NULL;
        mm_free_last = NULL;
    }
    // Curr is the first block
    else if (curr == mm_free_first) {
        mm_free_first = curr->next;
        curr->next->prev = NULL;
    }
    // Curr is the last block
    else if (curr == mm_free_last) {
        mm_free_last = curr->prev;
        curr->prev->next = NULL;
    }
    // Curr is a block in the middle of the linked list
    else {
        curr->next->prev = curr->prev;
        curr->prev->next = curr->next;
    }
    return curr;
}

/**
 * Finds the first free block in the heap with at least the given size.
 * If no block is large enough, returns NULL.
 */
static free_block_t *find_fit(size_t size) {
    for (free_block_t *curr = mm_free_first; mm_free_last != NULL && curr != NULL;
         curr = curr->next) {
        size_t curr_size = get_size((block_t *) curr);

        if (curr_size >= size) {
            return remove_from_freelist(curr);
        }
    }
    return NULL;
}

/** Gets the header corresponding to a given payload pointer */
static block_t *block_from_payload(void *ptr) {
    return ptr - offsetof(block_t, payload);
}

/**
 * mm_init - Initializes the allocator state
 */
bool mm_init(void) {
    // We want the first payload to start at ALIGNMENT bytes from the start of the heap
    void *padding = mem_sbrk(ALIGNMENT - sizeof(block_t));
    if (padding == (void *) -1) {
        return false;
    }

    // Initialize the heap with no blocks
    mm_heap_first = NULL;
    mm_heap_last = NULL;
    mm_free_first = NULL;
    mm_free_last = NULL;
    return true;
}

static void *split_block_exp(block_t *block, size_t size, size_t block_size) {
    set_header(block, size, true);
    set_footer(block, size, true);

    free_block_t *freed_block = (free_block_t *) (((void *) block) + size);
    set_header((block_t *) freed_block, block_size - size, false);
    set_footer((block_t *) freed_block, block_size - size, false);
    add_to_freelist((block_t *) freed_block);

    if ((block_t *) block == mm_heap_last) {
        mm_heap_last = ((void *) block) + size;
    }
    return ((block_t *) block)->payload;
}

static block_t *coalesce_left(block_t *block) {
    block_t *return_block = block;
    size_t block_size = get_size(block);
    size_t new_blocksize = block_size;
    size_t prev_size = 0;
    block_t *prev = NULL;

    if (block != mm_heap_first) {
        size_t *footer = (size_t *) (((void *) block) - sizeof(size_t));
        prev_size = *footer & ~1;
        prev = (block_t *) (((void *) block) - prev_size);
    }

    if (block != mm_heap_first && !is_allocated(prev)) {
        new_blocksize = prev_size + block_size;
        remove_from_freelist((free_block_t *) prev);

        if (block == mm_heap_last) {
            mm_heap_last = prev;
        }
        return_block = prev;
    }

    set_header(return_block, new_blocksize, false);
    set_footer(return_block, new_blocksize, false);
    return return_block;
}

static block_t *coalesce_right(block_t *block) {
    block_t *return_block = block;
    size_t block_size = get_size(block);
    size_t new_blocksize = block_size;
    size_t next_size = 0;
    block_t *next = NULL;

    // Get the size of the block to the right and the block itself
    if (block != mm_heap_last) {
        next = (block_t *) (((void *) block) + block_size);
        next_size = get_size(next);
    }

    // (block + next) coalesced
    if (block != mm_heap_last && !is_allocated(next)) {
        new_blocksize = block_size + next_size;
        remove_from_freelist((free_block_t *) next);

        if (next == mm_heap_last) {
            mm_heap_last = block;
        }
        return_block = block;
    }
    set_header(return_block, new_blocksize, false);
    set_footer(return_block, new_blocksize, false);
    return return_block;
}

/**
 * mm_malloc - Allocates a block with the given size
 */
void *mm_malloc(size_t size) {
    // The block must have enough space for a header and be 16-byte aligned
    size = round_up(sizeof(block_t) + size + sizeof(size_t), ALIGNMENT);

    // If there is a large enough free block, use it
    block_t *block = (block_t *) find_fit(size);
    if (block != NULL) {
        size_t block_size = get_size(block);
        if (block_size >= round_up(size + sizeof(free_block_t), ALIGNMENT)) {
            return split_block_exp(block, size, block_size);
        }
        else {
            set_header(block, block_size, true);
            set_footer(block, block_size, true);
            return block->payload;
        }
    }

    // Otherwise, a new block needs to be allocated at the end of the heap
    block = mem_sbrk(size);
    if (block == (void *) -1) {
        return NULL;
    }

    // Update mm_heap_first and mm_heap_last since we extended the heap
    if (mm_heap_first == NULL) {
        mm_heap_first = block;
    }
    mm_heap_last = block;

    // Initialize the block with the allocated size
    set_header(block, size, true);
    set_footer(block, size, true);
    return block->payload;
}

/**
 * mm_free - Releases a block to be reused for future allocations
 */
void mm_free(void *ptr) {
    // mm_free(NULL) does nothing
    if (ptr == NULL) {
        return;
    }

    // Mark the block as unallocated
    block_t *block = block_from_payload(ptr);
    block = coalesce_left(block);
    block = coalesce_right(block);
    add_to_freelist(block);
}

/**
 * mm_realloc - Change the size of the block by mm_mallocing a new block,
 *      copying its data, and mm_freeing the old block.
 */
void *mm_realloc(void *old_ptr, size_t size) {
    if (old_ptr == NULL) {
        return mm_malloc(size);
    }
    else if (size == 0) {
        mm_free(old_ptr);
        return NULL;
    }

    size_t old_size = get_size(block_from_payload(old_ptr)) - 2 * sizeof(block_t);
    void *new_ptr;
    if (old_size < size) {
        new_ptr = mm_malloc(size);
        memcpy(new_ptr, old_ptr, old_size);
    }
    else {
        new_ptr = mm_malloc(size);
        memcpy(new_ptr, old_ptr, size);
    }

    mm_free(old_ptr);
    return new_ptr;
}

/**
 * mm_calloc - Allocate the block and set it to zero.
 */
void *mm_calloc(size_t nmemb, size_t size) {
    void *new_ptr = mm_malloc(size * nmemb);
    memset(new_ptr, 0, size * nmemb);
    return new_ptr;
}

/**
 * mm_checkheap - So simple, it doesn't need a checker!
 */
void mm_checkheap(void) {
}
