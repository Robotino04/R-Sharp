/*
executionExitCode: -4
*/

[extern] calloc(num_items: i64, size_of_item: i64): *c_void;
[extern] free(pointer: *c_void): c_void;

// test library
[extern] set_i8_at_address(address: *i8, value: i8): c_void;
[extern] set_i16_at_address(address: *i16, value: i16): c_void;
[extern] set_i32_at_address(address: *i32, value: i32): c_void;
[extern] set_i64_at_address(address: *i64, value: i64): c_void;

main(): i32 {
    base: *c_void = calloc(1, 8);
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
