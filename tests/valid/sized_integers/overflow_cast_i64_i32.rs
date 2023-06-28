/*
executionExitCode: 1
*/

foo(): i64{
    // 2**32+1
    return 4294967297;
}

main(): i32 {
    return foo();
}