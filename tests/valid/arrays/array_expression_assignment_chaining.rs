/*
executionExitCode: 5
*/

main(): i32 {
    a: [i64, 3];
    b: [i64, 3];

    a = b = [1, 2, 3];
    return a[1] + b[2];
}