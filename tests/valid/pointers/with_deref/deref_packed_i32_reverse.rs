/*
executionExitCode: 0
*/

[extern] malloc(size: i64): *c_void;
[extern] memset(pointer: *c_void, value: i32, size: i64): *c_void;
[extern] free(pointer: *c_void): c_void;

main(): i32 {
    base: *c_void = malloc(8);
    memset(base, 255, 8);
    a: *i32 = base;
    b: *i32 = base - -4;
    *b = -123;
    *a = 123;
    return *a + *b;
}
