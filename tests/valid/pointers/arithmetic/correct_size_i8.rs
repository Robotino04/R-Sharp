/*
executionExitCode: 0
*/

malloc @ std::libc;

main(): i32{
    base: *c_void = malloc(16);
    a: *i8 = base;         // base + 0
    b: *i8 = a + 2;        // base + 2
    c: *i8 = base + 2;     // base + 2

    *a = 6;
    *b = 17;
    *c = 99;
    return *b == 17;
}