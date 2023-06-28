/*
executionExitCode: 3
*/

foo(): i64{
    // 2**16+3
    return 65539;
}

bar(): i16{
    return foo();
}

main(): i32 {
    return bar();
}