#include <stdio.h>
#include <stdlib.h>

int main() {
    char *buf = malloc(123);
    for (size_t i = 0;; i++) {
        ((volatile char *) buf)[i];
        printf("%zu\n", i);
    }
}

// 11

// stdout: 0
// stdout: 1
// stdout: 2
// stdout: 3
// stdout: 4
// stdout: 5
// stdout: 6
// stdout: 7
// stdout: 8
// stdout: 9
// stdout: 10
// stdout: 11
// stdout: 12
// stdout: 13
// stdout: 14
// stdout: 15
// stdout: 16
// stdout: 17
// stdout: 18
// stdout: 19
// stdout: 20
// stdout: 21
// stdout: 22
// stdout: 23
// stdout: 24
// stdout: 25
// stdout: 26
// stdout: 27
// stdout: 28
// stdout: 29
// stdout: 30
// stdout: 31
// stdout: 32
// stdout: 33
// stdout: 34
// stdout: 35
// stdout: 36
// stdout: 37
// stdout: 38
// stdout: 39
// stdout: 40
// stdout: 41
// stdout: 42
// stdout: 43
// stdout: 44
// stdout: 45
// stdout: 46
// stdout: 47
// stdout: 48
// stdout: 49
// stdout: 50
// stdout: 51
// stdout: 52
// stdout: 53
// stdout: 54
// stdout: 55
// stdout: 56
// stdout: 57
// stdout: 58
// stdout: 59
// stdout: 60
// stdout: 61
// stdout: 62
// stdout: 63
// stdout: 64
// stdout: 65
// stdout: 66
// stdout: 67
// stdout: 68
// stdout: 69
// stdout: 70
// stdout: 71
// stdout: 72
// stdout: 73
// stdout: 74
// stdout: 75
// stdout: 76
// stdout: 77
// stdout: 78
// stdout: 79
// stdout: 80
// stdout: 81
// stdout: 82
// stdout: 83
// stdout: 84
// stdout: 85
// stdout: 86
// stdout: 87
// stdout: 88
// stdout: 89
// stdout: 90
// stdout: 91
// stdout: 92
// stdout: 93
// stdout: 94
// stdout: 95
// stdout: 96
// stdout: 97
// stdout: 98
// stdout: 99
// stdout: 100
// stdout: 101
// stdout: 102
// stdout: 103
// stdout: 104
// stdout: 105
// stdout: 106
// stdout: 107
// stdout: 108
// stdout: 109
// stdout: 110
// stdout: 111
// stdout: 112
// stdout: 113
// stdout: 114
// stdout: 115
// stdout: 116
// stdout: 117
// stdout: 118
// stdout: 119
// stdout: 120
// stdout: 121
// stdout: 122

// stderr: Invalid heap access: address 0x100002000 is not in an allocation or was already freed
// stderr: at bin/hello_overflow2(main+0x2d)
// stderr: main
// stderr: hello_overflow2.c:7
