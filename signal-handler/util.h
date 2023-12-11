#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define assert(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "Assertion failed: '%s' at %s:%d\n", #cond, __FILE__, __LINE__); \
        _exit(1); \
    } \
} while (0)

__attribute__((__noreturn__))
void report_seg_fault(void *address);

__attribute__((__noreturn__))
void report_invalid_heap_access(void *address);

__attribute__((__noreturn__))
void report_invalid_free(void *address);

__attribute__((__noreturn__))
void report_double_free(void *allocation, size_t allocation_size);

__attribute__((__noreturn__))
void report_memory_leak(void *allocation, size_t allocation_size);

#endif /* UTIL_H */
