/*
executionExitCode: 5
*/

foo(): i32{
    return 5;
}

bar(): i64{
    return foo();
}

main(): i32 {
    return bar();
}