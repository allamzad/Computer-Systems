#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *x = malloc(4096 * 100);
    printf("Hello World!\n");
    free(x);
    printf("Hello World!\n");
    ((volatile char *) x)[1234 + 4096 * 99];
    printf("Hello World!\n");
}

// 11

// stdout: Hello World!
// stdout: Hello World!

// stderr: Invalid heap access: address 0x1000644d2 is not in an allocation or was already freed
// stderr: at bin/hello_use_after_free5(main+0x48)
// stderr: main
// stderr: hello_use_after_free5.c:10
