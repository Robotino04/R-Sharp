/*
executionExitCode: 5
*/

fib(n: i64): i64 {
    if (n == 0 || n == 1) {
        return n;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

main(): i32 {
    n: i64 = 5;
    return fib(n);
}