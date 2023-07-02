/*
executionExitCode: 1
*/

foo(): i16{
    return -5;
}

bar(): i64{
    return foo();
}

main(): i32 {
    return bar()/(-5);
}