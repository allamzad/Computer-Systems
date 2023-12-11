#include <stdlib.h>

int main() {
    void *x = malloc(100);
    free(x);
    *(volatile char *) NULL = 5;
}

// 10

// stderr: Segmentation fault: unknown address (nil)
// stderr: at bin/hello_access_null(main+0x25)
// stderr: main
// stderr: hello_access_null.c:6
