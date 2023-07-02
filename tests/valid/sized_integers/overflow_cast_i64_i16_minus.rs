/*
executionExitCode: 0
*/

foo(): i64{
    // -2**16
    return -65536;
}

bar(): i16{
    return foo();
}

main(): i32 {
    return bar();
}