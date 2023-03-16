/*
executionExitCode: 3
*/

main(): i32 {
    a: i64 = 0;
    a || (a = 3) || (a = 4);
    return a;
}