#include <stdio.h>
#include <stdlib.h>

int main() {
    char *buf = malloc(4096);
    for (size_t i = 4000;; i++) {
        ((volatile char *) buf)[i];
        printf("%zu\n", i);
    }
}

// 11

// stdout: 4000
// stdout: 4001
// stdout: 4002
// stdout: 4003
// stdout: 4004
// stdout: 4005
// stdout: 4006
// stdout: 4007
// stdout: 4008
// stdout: 4009
// stdout: 4010
// stdout: 4011
// stdout: 4012
// stdout: 4013
// stdout: 4014
// stdout: 4015
// stdout: 4016
// stdout: 4017
// stdout: 4018
// stdout: 4019
// stdout: 4020
// stdout: 4021
// stdout: 4022
// stdout: 4023
// stdout: 4024
// stdout: 4025
// stdout: 4026
// stdout: 4027
// stdout: 4028
// stdout: 4029
// stdout: 4030
// stdout: 4031
// stdout: 4032
// stdout: 4033
// stdout: 4034
// stdout: 4035
// stdout: 4036
// stdout: 4037
// stdout: 4038
// stdout: 4039
// stdout: 4040
// stdout: 4041
// stdout: 4042
// stdout: 4043
// stdout: 4044
// stdout: 4045
// stdout: 4046
// stdout: 4047
// stdout: 4048
// stdout: 4049
// stdout: 4050
// stdout: 4051
// stdout: 4052
// stdout: 4053
// stdout: 4054
// stdout: 4055
// stdout: 4056
// stdout: 4057
// stdout: 4058
// stdout: 4059
// stdout: 4060
// stdout: 4061
// stdout: 4062
// stdout: 4063
// stdout: 4064
// stdout: 4065
// stdout: 4066
// stdout: 4067
// stdout: 4068
// stdout: 4069
// stdout: 4070
// stdout: 4071
// stdout: 4072
// stdout: 4073
// stdout: 4074
// stdout: 4075
// stdout: 4076
// stdout: 4077
// stdout: 4078
// stdout: 4079
// stdout: 4080
// stdout: 4081
// stdout: 4082
// stdout: 4083
// stdout: 4084
// stdout: 4085
// stdout: 4086
// stdout: 4087
// stdout: 4088
// stdout: 4089
// stdout: 4090
// stdout: 4091
// stdout: 4092
// stdout: 4093
// stdout: 4094
// stdout: 4095

// stderr: Invalid heap access: address 0x100002000 is not in an allocation or was already freed
// stderr: at bin/hello_overflow3(main+0x2d)
// stderr: main
// stderr: hello_overflow3.c:7
