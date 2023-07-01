/*
executionExitCode: 2
*/

malloc, memset @ std::libc;

main(): i32 {
    a: *i32 = malloc(4);
    memset(a, 255, 4);
    *a = 2;
    return *a;
}