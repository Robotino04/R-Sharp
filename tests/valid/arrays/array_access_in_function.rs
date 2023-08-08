/*
executionExitCode: 3
*/

foo(): i64{
    // this will prob. segfault if the array does stupid stack manipulation
    return [1, 3, 8][1];
}

main(): i32 {
    return foo();
}