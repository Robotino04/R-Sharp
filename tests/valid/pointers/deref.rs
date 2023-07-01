/*
executionExitCode: 0
*/

malloc, memset @ std::libc;

main(): i32 {
    a: *i32 = malloc(4);
    memset(a, 0, 4);
    return *a;
}
