/*
executionExitCode: 2
*/

main(): i32 {
    a: i64 = 0;
    b: i64 = 0;
    a ? (b = 1) : (b = 2);
    return b;
}