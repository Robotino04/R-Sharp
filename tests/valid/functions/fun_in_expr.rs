/*
executionExitCode: 16
*/

sum(a: i64, b: i64): i64 {
    return a + b;
}

main(): i32 {
    a: i64 = sum(1, 2) - (sum(1, 2) / 2) * 2;
    b: i64 = 2*sum(3, 4) + sum(1, 2);
    return b - a;
}