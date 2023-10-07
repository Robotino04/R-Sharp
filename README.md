# R# (R-Sharp)
[![CMake](https://github.com/Robotino04/R-Sharp/actions/workflows/cmake.yml/badge.svg)](https://github.com/Robotino04/R-Sharp/actions/workflows/cmake.yml)

A general purpose programming language.


## Unit Tests
Some of the tests used for R# are from [this](https://github.com/nlsandler/write_a_c_compiler) repository and rewritten to R#.


## Register allocation AArch64

| Register | Call Preserved | Purpose |
|----------|----------------|---------|
| r0       | No             | Result, function return, first argument |
| r1       | No             | Secondary Result, second argument |
| r2       | No             | General Purpose, third argument |
| r3       | No             | General Purpose, fourth argument |
| r4       | No             | General Purpose, fifth argument |
| r5       | No             | General Purpose, sixth argument |
| r6       | No             | General Purpose, seventh argument |
| r7       | No             | General Purpose, eight argument |
| r8       | No             | General Purpose |
| r9       | No             | General Purpose |
| r10      | No             | General Purpose |
| r11      | No             | General Purpose |
| r12      | No             | General Purpose |
| r13      | No             | General Purpose |
| r14      | No             | General Purpose |
| r15      | No             | General Purpose |
| r16      | No             | General Purpose |
| r17      | No             | General Purpose |
| r18      | No             | Platform Register (DON'T USE!) |
| r19      | Yes            | preserve x0 for internal function calls |
| r20      | Yes            | preserve stack alignment for function calls |
| r21      | Yes            | General Purpose |
| r22      | Yes            | General Purpose |
| r23      | Yes            | General Purpose |
| r24      | Yes            | General Purpose |
| r25      | Yes            | General Purpose |
| r26      | Yes            | General Purpose |
| r27      | Yes            | General Purpose |
| r28      | Yes            | General Purpose |
| r29      | Yes            | Frame Pointer   |
| r30      | Yes            | Link Register   |



## Register allocation x86_64

| Register   | Call Preserved | Purpose |
|------------|----------------|---------|
| rax        | No             | Result, function return |
| rbx        | Yes            | Secondary Result, preserve x0 for internal function calls |
| rcx        | No             | Fourth Argument |
| rdx        | No             | Thrid Argument |
| rsi        | No             | Second Argument |
| rdi        | No             | First argument |
| rbp        | Yes            | Frame Pointer |
| rsp        | Yes            | Stack Pointer |
| r8         | No             | Fifth Argument |
| r9         | No             | Sixth Argument |
| r10        | No             | General Purpose |
| r11        | No             | General Purpose |
| r12        | Yes            | preserve stack alignment for function calls |
| r13        | Yes            | General Purpose |
| r14        | Yes            | General Purpose |
| r15        | Yes            | General Purpose |
<!-- | ymm0       | No             | General Purpose |
| ymm1       | No             | General Purpose |
| ymm2       | No             | General Purpose |
| ymm3       | No             | General Purpose |
| ymm4       | No             | General Purpose |
| ymm5       | No             | General Purpose |
| ymm6       | No             | General Purpose |
| ymm7       | No             | General Purpose |
| ymm8       | No             | General Purpose |
| ymm9       | No             | General Purpose |
| ymm10      | No             | General Purpose |
| ymm11      | No             | General Purpose |
| ymm12      | No             | General Purpose |
| ymm13      | No             | General Purpose |
| ymm14      | No             | General Purpose |
| ymm15      | No             | General Purpose | -->

# References
- x86_64 reference: <https://www.felixcloutier.com/x86/>