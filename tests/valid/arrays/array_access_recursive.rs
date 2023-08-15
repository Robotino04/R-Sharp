/*
executionExitCode: 5
*/

main(): i32 {
    a: [i64, 3] = [1, 9, 0];
    b: [[i64, 2], 3] = [[77, 77], [5, 88], [99, 99]];

    return b[a[0]][a[2]];
}