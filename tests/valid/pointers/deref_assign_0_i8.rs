/*
executionExitCode: 0
*/

malloc, memset @ std::libc;

main(): i32 {
    a: *i8 = malloc(1);
    memset(a, 255, 1);
    *a = 0;
    return *a;
}