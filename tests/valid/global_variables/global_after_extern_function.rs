/*
executionExitCode: 7
*/

[extern] bar(): c_void;

foo: i64 = 4;

main(): i32 {
    return foo + 3;
}