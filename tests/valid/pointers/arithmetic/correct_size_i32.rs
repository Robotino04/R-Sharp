/*
executionExitCode: 0
*/

malloc @ std::libc;

main(): i32{
    base: *c_void = malloc(16);
    a: *i32 = base;         // base + 0
    b: *i32 = a + 2;        // base + 8
    c: *i32 = base + 8;     // base + 8

    *a = 6;
    *b = 17;
    *c = 99;
    return *b == 17;
}