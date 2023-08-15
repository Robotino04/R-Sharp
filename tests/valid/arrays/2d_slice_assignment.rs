/*
executionExitCode: 2
*/

main(): i32 {
    a: [[i64, 3], 4];

    a[2] = [1, 2, 3];
    return a[2][1];
}