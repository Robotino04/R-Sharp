/*
executionExitCode: 0
*/

malloc @ std::libc;

main(): i32{
    base: *c_void = malloc(16);
    a: *i16 = base;         // base + 0
    b: *i16 = a + 2;        // base + 4
    c: *i16 = base + 4;     // base + 4

    *a = 6;
    *b = 17;
    *c = 99;
    return *b == 17;
}