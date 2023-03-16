/*
executionExitCode: 2
*/

foo(x: i64): i64 {
    return x + 1;
}

main(): i32 {
    a: i64 = 1;
    return foo(a);
}
