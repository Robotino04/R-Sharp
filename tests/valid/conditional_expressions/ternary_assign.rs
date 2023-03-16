/*
executionExitCode: 1
*/

// This is valid, bacause R# uses the C++ ternary operator.

main(): i32 {
    a: i64 = 2;
    b: i64 = 1;
    a > b ? a = 1 : a = 0;
    return a;
}