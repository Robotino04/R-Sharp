/*
executionExitCode: 9
*/

main(): i32 {
    a: i64 = 4;

    ptr: *i64 = $a;
    return *ptr = 9;
}
