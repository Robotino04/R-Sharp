/*
executionExitCode: 2
*/

malloc, memset @ std::libc;

main(): i32 {
    a: *i64 = malloc(8);
    memset(a, 255, 8);
    *a = 2;
    return *a;
}