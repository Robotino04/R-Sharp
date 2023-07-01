/*
executionExitCode: 8
*/

some_function @ self_recursive_no_lib;

some_function(): i64{
    return 8;
}

main(): i32 {
    return some_function();
}
