/*
executionExitCode: 0
*/

malloc, memset @ std::libc;

main(): i32 {
    base: *c_void = malloc(8);
    memset(base, 255, 8);
    a: *i16 = base;
    b: *i16 = base - -2;
    c: *i16 = base - -4;
    d: *i16 = base - -6;
    *d = -8067;
    *c = 8167;
    *b = -223;
    *a = 123;
    return *a + *b + *c + *d;
}
