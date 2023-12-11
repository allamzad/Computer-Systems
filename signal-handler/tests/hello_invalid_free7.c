#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *x = malloc(100);
    strcpy(x, "Hello World!");
    printf("%s\n", x);
    free(x + 8192);
}

// 20

// stdout: Hello World!

// stderr: Invalid free(): 0x100003f9c is not an allocation
// stderr: at bin/hello_invalid_free7(main+0x53)
// stderr: main
// stderr: hello_invalid_free7.c:9
