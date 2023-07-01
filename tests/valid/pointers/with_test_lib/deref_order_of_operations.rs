/*
executionExitCode: 108
*/

malloc, memset @ std::libc;

set_i32_at_address @ std::test::pointers;

main(): i32 {
    a: *i32 = malloc(4);
    b: *i32 = malloc(4);
    set_i32_at_address(a, -2);
    set_i32_at_address(b, -54);
    return *a**b;
}
