/*
executionExitCode: 122
*/

malloc @ std::libc;

main(): i32{
    base: *c_void = malloc(4);
    a: *i16 = base + 4;
    *(a-1) = 123;
    *(a-2) = -1;
    return *(a-1) + *(a-2);
}