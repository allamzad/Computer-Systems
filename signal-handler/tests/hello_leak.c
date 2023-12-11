#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *x = malloc(100);
    strcpy(x, "Hello World!");
    printf("%s\n", x);
}

// 30

// stdout: Hello World!

// stderr: Memory leak: allocation of 100 bytes at 0x100001f9c was never freed
