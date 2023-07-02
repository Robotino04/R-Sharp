/*
executionExitCode: 0
*/

foo(): i64{
    // -2**32
    return -4294967296;
}

main(): i32 {
    return foo();
}