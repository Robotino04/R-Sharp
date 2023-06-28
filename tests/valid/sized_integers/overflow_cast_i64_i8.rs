/*
executionExitCode: 1
*/

foo(): i64{
    // 2**8+1
    return 257;
}

bar(): i8{
    return foo();
}

main(): i32 {
    return bar();
}