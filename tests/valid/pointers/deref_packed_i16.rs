/*
executionExitCode: 0
*/

[extern] malloc(size: i64): *c_void;
[extern] memset(pointer: *c_void, value: i32, size: i64): *c_void;
[extern] free(pointer: *c_void): c_void;

// test library
[extern] set_i8_at_address(address: *i8, value: i8): c_void;
[extern] set_i16_at_address(address: *i16, value: i16): c_void;
[extern] set_i32_at_address(address: *i32, value: i32): c_void;
[extern] set_i64_at_address(address: *i64, value: i64): c_void;

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
