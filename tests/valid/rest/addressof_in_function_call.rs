/*
executionExitCode: 3
*/

foo(_: *i32): i32{
    return 2;
}

main(): i32 {
    index: i32;
    index = 1 + foo($index);
    return index;
}