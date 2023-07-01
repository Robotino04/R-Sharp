/*
executionExitCode: 108
*/

malloc, memset @ std::libc;


main(): i32 {
    a: *i32 = malloc(4);
    b: *i32 = malloc(4);
    *a = -2;
    *b = -54;
    return *a**b;
}
