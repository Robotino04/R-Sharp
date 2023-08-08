/*
compilationExitCode: 0
*/

malloc, memset @ std::libc;

main(): i32 {
    a: *[i64, 3] = malloc(3*8);
    memset(a, 0, 3*8);

    return (*a)[0] + (*a)[1] + (*a)[2];
}