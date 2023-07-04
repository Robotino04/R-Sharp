/*
executionExitCode: 122
*/

malloc @ std::libc;

main(): i32{
    a: *i16 = malloc(4);
    *(a+0) = 123;
    *(a+1) = -1;
    return *(a+0) + *(a+1);
}