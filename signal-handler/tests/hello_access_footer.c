#include <stdio.h>
#include <stdlib.h>

int main() {
    void *x1 = malloc(1);
    void *x2 = malloc(2);
    printf("Hello World!\n");
    *(volatile char *) (x2 + 4097);
    printf("Hello World!\n");
    free(x1);
    free(x2);
}

// 11

// stdout: Hello World!

// stderr: Invalid heap access: address 0x100004fff is not in an allocation or was already freed
// stderr: at bin/hello_access_footer(main+0x39)
// stderr: main
// stderr: hello_access_footer.c:8
