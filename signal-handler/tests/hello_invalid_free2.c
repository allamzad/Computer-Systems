#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *x = malloc(100);
    strcpy(x, "Hello World!");
    printf("%s\n", x);
    free(x + 1);
}

// 20

// stdout: Hello World!

// stderr: Invalid free(): 0x100001f9d is not an allocation
// stderr: at bin/hello_invalid_free2(main+0x53)
// stderr: main
// stderr: hello_invalid_free2.c:9
