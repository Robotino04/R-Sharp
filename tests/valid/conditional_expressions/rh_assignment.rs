/*
executionExitCode: 1
*/

main(): i32 {
    flag: i64 = 1;
    a: i64 = 0;
    flag ? a = 1 : (a = 0);
    return a;
}