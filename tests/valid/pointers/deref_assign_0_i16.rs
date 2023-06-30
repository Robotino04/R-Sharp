/*
executionExitCode: 0
*/

[extern] malloc(size: i64): *c_void;
[extern] memset(pointer: *c_void, value: i32, size: i64): *c_void;
[extern] free(pointer: *c_void): c_void;

main(): i32 {
    a: *i16 = malloc(2);
    memset(a, 255, 2);
    *a = 0;
    return *a;
}