#include <stdio.h>
#include <stdlib.h>

int main() {
    void *x1 = malloc(1);
    void *x2 = malloc(2);
    printf("Hello World!\n");
    *(volatile char *) (x1 - 8192);
    printf("Hello World!\n");
    free(x1);
    free(x2);
}

// 10

// stdout: Hello World!

// stderr: Segmentation fault: unknown address 0xffffffff
// stderr: at bin/hello_access_before_heap(main+0x39)
// stderr: main
// stderr: hello_access_before_heap.c:8
