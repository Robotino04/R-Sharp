/*
executionExitCode: 108
*/

[extern] calloc(num_items: i64, size_of_item: i64): *c_void;
[extern] free(pointer: *c_void): c_void;

// test library
[extern] set_i8_at_address(address: *i8, value: i8): c_void;
[extern] set_i16_at_address(address: *i16, value: i16): c_void;
[extern] set_i32_at_address(address: *i32, value: i32): c_void;
[extern] set_i64_at_address(address: *i64, value: i64): c_void;

main(): i32 {
    a: *i32 = calloc(1, 4);
    b: *i32 = calloc(1, 4);
    set_i32_at_address(a, -2);
    set_i32_at_address(b, -54);
    return *a**b;
}
