/*
executionExitCode: 0
*/

[extern] calloc(num_items: i64, size_of_item: i64): *c_void;
[extern] free(pointer: *c_void): c_void;

main(): i32 {
    a: *i32 = calloc(1, 4);
    return *a;
}
