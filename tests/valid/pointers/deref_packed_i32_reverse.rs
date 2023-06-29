/*
executionExitCode: 0
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
    a: *i32 = base;
    b: *i32 = base - -4;
    set_i32_at_address(b, -123);
    set_i32_at_address(a, 123);
    return *a + *b;
}
