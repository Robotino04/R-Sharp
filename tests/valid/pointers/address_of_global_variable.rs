/*
executionExitCode: 8
*/

global: i64 = 5;

main(): i32 {
    a: *i64 = $global;
    *a = 8;
    return global;
}
