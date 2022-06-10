/*
fail: 1
*/

foo(a: int): int {
    return a + 1;
}

main(): int {
    return foo(1, 2);
}