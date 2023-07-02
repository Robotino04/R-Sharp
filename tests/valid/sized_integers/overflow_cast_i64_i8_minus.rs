/*
executionExitCode: 0
*/

foo(): i64{
    // -2**8
    return -256;
}

bar(): i8{
    return foo();
}

main(): i32 {
    return bar();
}