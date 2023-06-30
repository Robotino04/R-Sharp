/*
executionExitCode: 2
*/

[extern] malloc(size: i64): *c_void;
[extern] memset(pointer: *c_void, value: i32, size: i64): *c_void;
[extern] free(pointer: *c_void): c_void;

main(): i32 {
    a: *i8 = malloc(1);
    memset(a, 255, 1);
    *a = 2;
    return *a;
}