/*
executionExitCode: 0
*/

malloc, memset @ std::libc;

set_i16_at_address @ std::test::pointers;

main(): i32 {
    base: *c_void = malloc(8);
    memset(base, 255, 8);
    a: *i16 = base;
    b: *i16 = base - -2;
    c: *i16 = base - -4;
    d: *i16 = base - -6;
    set_i16_at_address(a, 123);
    set_i16_at_address(b, -223);
    set_i16_at_address(c, 8167);
    set_i16_at_address(d, -8067);
    return *a + *b + *c + *d;
}
