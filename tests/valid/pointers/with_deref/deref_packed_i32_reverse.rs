/*
executionExitCode: 0
*/

malloc, memset @ std::libc;

main(): i32 {
    base: *c_void = malloc(8);
    memset(base, 255, 8);
    a: *i32 = base;
    b: *i32 = base - -4;
    *b = -123;
    *a = 123;
    return *a + *b;
}
