/*
executionExitCode: -4
*/

malloc, memset @ std::libc;

set_i8_at_address @ std::test::pointers;

main(): i32 {
    base: *c_void = malloc(8);
    memset(base, 255, 8);
    a: *i8 = base;
    b: *i8 = base - -1;
    c: *i8 = base - -2;
    d: *i8 = base - -3;
    e: *i8 = base - -4;
    f: *i8 = base - -5;
    g: *i8 = base - -6;
    h: *i8 = base - -7;
    set_i8_at_address(h, -8);
    set_i8_at_address(g, 7);
    set_i8_at_address(f, -6);
    set_i8_at_address(e, 5);
    set_i8_at_address(d, -4);
    set_i8_at_address(c, 3);
    set_i8_at_address(b, -2);
    set_i8_at_address(a, 1);
    return *a + *b + *c + *d + *e + *f + *g + *h;
}
