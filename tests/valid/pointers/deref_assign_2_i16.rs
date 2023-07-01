/*
executionExitCode: 2
*/

malloc, memset @ std::libc;

main(): i32 {
    a: *i16 = malloc(2);
    memset(a, 255, 2);
    *a = 2;
    return *a;
}