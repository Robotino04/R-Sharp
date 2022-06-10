/*
return: 5
*/

fib(n: int): int {
    if (n == 0 || n == 1) {
        return n;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

main(): int {
    n: int = 5;
    return fib(n);
}