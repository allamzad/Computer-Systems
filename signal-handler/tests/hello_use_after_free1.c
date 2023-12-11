#include <stdlib.h>
#include <string.h>

int main() {
    char *x = malloc(100);
    free(x);
    strcpy(x, "Hello World!");
}

// 11

// stderr: Invalid heap access: address 0x100001f9c is not in an allocation or was already freed
// stderr: at /lib/x86_64-linux-gnu/libc.so.6(+0x189f58)
// stderr: ??
// stderr: strcpy-avx2.S:612
// stderr: at bin/hello_use_after_free1(main+0x32)
// stderr: main
// stderr: hello_use_after_free1.c:7
