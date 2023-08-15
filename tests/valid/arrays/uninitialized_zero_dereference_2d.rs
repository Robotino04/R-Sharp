/*
compilationExitCode: 0
*/

malloc, memset @ std::libc;

main(): i32 {
    a: *[[i64, 3], 4] = malloc(4*3*8);
    memset(a, 0, 4*3*8);

    return (*a)[0][0] + (*a)[1][0] + (*a)[2][0] + (*a)[3][0]
        +  (*a)[0][1] + (*a)[1][1] + (*a)[2][1] + (*a)[3][1]
        +  (*a)[0][2] + (*a)[1][2] + (*a)[2][2] + (*a)[3][2];
}