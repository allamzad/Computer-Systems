#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *x = malloc(8192);
    strcpy(x, "Hello World!");
    printf("%s\n", x);
    free(x + 4096);
}

// 20

// stdout: Hello World!

// stderr: Invalid free(): 0x100002000 is not an allocation
// stderr: at bin/hello_invalid_free1(main+0x53)
// stderr: main
// stderr: hello_invalid_free1.c:9
