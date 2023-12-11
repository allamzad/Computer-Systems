#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *x = malloc(sizeof("Hello World!"));
    strcpy(x, "Hello World!");
    printf("%s\n", x);
    free(x);
    *(volatile char *) (x - 4096);
}

// 11

// stdout: Hello World!

// stderr: Invalid heap access: address 0x100000ff3 is not in an allocation or was already freed
// stderr: at bin/hello_header_after_free(main+0x4f)
// stderr: main
// stderr: hello_header_after_free.c:10
