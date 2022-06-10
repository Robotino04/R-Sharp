/*
return: 16
*/

sum(a: int, b: int): int {
    return a + b;
}

main(): int {
    a: int = sum(1, 2) - (sum(1, 2) / 2) * 2;
    b: int = 2*sum(3, 4) + sum(1, 2);
    return b - a;
}