#include <stdio.h>
#include <stdlib.h>

void b(volatile char *buf, size_t i) {
    buf[i];
    printf("b: %zu\n", i);
}

void a(char *buf, size_t i) {
    printf("a: %zu\n", i);
    b(buf, i);
}

int main() {
    for (size_t i = 1; i <= 4096; i++) {
        free(malloc(i));
    }
    char *buf = malloc(12345);
    for (size_t i = 4096 * 100; i > 0; i -= 4096) {
        malloc(i);
    }
    for (size_t i = 12300;; i++) {
        a(buf, i);
    }
}

// 11

// stdout: a: 12300
// stdout: b: 12300
// stdout: a: 12301
// stdout: b: 12301
// stdout: a: 12302
// stdout: b: 12302
// stdout: a: 12303
// stdout: b: 12303
// stdout: a: 12304
// stdout: b: 12304
// stdout: a: 12305
// stdout: b: 12305
// stdout: a: 12306
// stdout: b: 12306
// stdout: a: 12307
// stdout: b: 12307
// stdout: a: 12308
// stdout: b: 12308
// stdout: a: 12309
// stdout: b: 12309
// stdout: a: 12310
// stdout: b: 12310
// stdout: a: 12311
// stdout: b: 12311
// stdout: a: 12312
// stdout: b: 12312
// stdout: a: 12313
// stdout: b: 12313
// stdout: a: 12314
// stdout: b: 12314
// stdout: a: 12315
// stdout: b: 12315
// stdout: a: 12316
// stdout: b: 12316
// stdout: a: 12317
// stdout: b: 12317
// stdout: a: 12318
// stdout: b: 12318
// stdout: a: 12319
// stdout: b: 12319
// stdout: a: 12320
// stdout: b: 12320
// stdout: a: 12321
// stdout: b: 12321
// stdout: a: 12322
// stdout: b: 12322
// stdout: a: 12323
// stdout: b: 12323
// stdout: a: 12324
// stdout: b: 12324
// stdout: a: 12325
// stdout: b: 12325
// stdout: a: 12326
// stdout: b: 12326
// stdout: a: 12327
// stdout: b: 12327
// stdout: a: 12328
// stdout: b: 12328
// stdout: a: 12329
// stdout: b: 12329
// stdout: a: 12330
// stdout: b: 12330
// stdout: a: 12331
// stdout: b: 12331
// stdout: a: 12332
// stdout: b: 12332
// stdout: a: 12333
// stdout: b: 12333
// stdout: a: 12334
// stdout: b: 12334
// stdout: a: 12335
// stdout: b: 12335
// stdout: a: 12336
// stdout: b: 12336
// stdout: a: 12337
// stdout: b: 12337
// stdout: a: 12338
// stdout: b: 12338
// stdout: a: 12339
// stdout: b: 12339
// stdout: a: 12340
// stdout: b: 12340
// stdout: a: 12341
// stdout: b: 12341
// stdout: a: 12342
// stdout: b: 12342
// stdout: a: 12343
// stdout: b: 12343
// stdout: a: 12344
// stdout: b: 12344
// stdout: a: 12345

// stderr: Invalid heap access: address 0x102005000 is not in an allocation or was already freed
// stderr: at bin/hello_overflow4(b+0x18)
// stderr: b
// stderr: hello_overflow4.c:5
// stderr: at bin/hello_overflow4(a+0x35)
// stderr: a
// stderr: hello_overflow4.c:12
// stderr: at bin/hello_overflow4(main+0x9b)
// stderr: main
// stderr: hello_overflow4.c:22
