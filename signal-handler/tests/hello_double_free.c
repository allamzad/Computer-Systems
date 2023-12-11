#include <stdio.h>
#include <stdlib.h>

int main() {
    char *x = malloc(100);
    printf("Hello World!\n");
    free(x);
    printf("Hello World!\n");
    free(x);
    printf("Hello World!\n");
}

// 21

// stdout: Hello World!
// stdout: Hello World!

// stderr: Double free(): allocation of 100 bytes at 0x100001f9c was already freed
// stderr: at bin/hello_double_free(main+0x50)
// stderr: main
// stderr: hello_double_free.c:10
