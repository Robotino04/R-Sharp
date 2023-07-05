/*
executionExitCode: 0
*/

malloc @ std::libc;

main(): i32{
    base: *c_void = malloc(32);
    a: *i64 = base;         // base + 0
    b: *i64 = a + 2;        // base + 16
    c: *i64 = base + 16;     // base + 16

    *a = 6;
    *b = 17;
    *c = 99;
    return *b == 17;
}