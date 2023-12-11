#include <stdio.h>
#include <stdlib.h>

int main() {
    void *x1 = malloc(1);
    void *x2 = malloc(2);
    printf("Hello World!\n");
    *(volatile char *) (x2 + 4098);
    printf("Hello World!\n");
    free(x1);
    free(x2);
}

// 10

// stdout: Hello World!

// stderr: Segmentation fault: unknown address 0x100005000
// stderr: at bin/hello_access_after_heap(main+0x39)
// stderr: main
// stderr: hello_access_after_heap.c:8
