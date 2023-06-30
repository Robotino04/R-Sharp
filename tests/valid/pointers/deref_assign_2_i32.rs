/*
executionExitCode: 2
*/

[extern] malloc(size: i64): *c_void;
[extern] memset(pointer: *c_void, value: i32, size: i64): *c_void;
[extern] free(pointer: *c_void): c_void;

main(): i32 {
    a: *i32 = malloc(4);
    memset(a, 255, 4);
    *a = 2;
    return *a;
}