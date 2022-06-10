/*
return: -1
*/

foo(a: int, b: int): int;

main(): int {
    return foo(1, 2);
}

foo(x: int, y: int): int{
    return x - y;
}