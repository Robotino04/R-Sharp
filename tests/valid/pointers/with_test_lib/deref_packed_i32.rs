/*
executionExitCode: 0
*/

malloc, memset @ std::libc;

set_i32_at_address @ std::test::pointers;

main(): i32 {
    base: *c_void = malloc(8);
    memset(base, 255, 8);
    a: *i32 = base;
    b: *i32 = base - -4;
    set_i32_at_address(a, 123);
    set_i32_at_address(b, -123);
    return *a + *b;
}
