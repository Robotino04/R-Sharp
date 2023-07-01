/*
executionExitCode: 2
*/

main(): i32 {
    a: i64 = 4;
    b: *i64 = $a;
    *b = 2;
    return a;
}
