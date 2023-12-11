#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    void *x = malloc(1000);
    (void) x;
    free((void *) 1000);
}

// 20

// stderr: Invalid free(): 0x3e8 is not an allocation
// stderr: at bin/hello_invalid_free6(main+0x20)
// stderr: main
// stderr: hello_invalid_free6.c:8
