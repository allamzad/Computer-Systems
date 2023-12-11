#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    void *x = malloc(10);
    (void) x;
    free((void *) 0x12345678);
}

// 20

// stderr: Invalid free(): 0x12345678 is not an allocation
// stderr: at bin/hello_invalid_free4(main+0x20)
// stderr: main
// stderr: hello_invalid_free4.c:8
