/*
return: 4
*/

foo(a: int): int;

main(): int{
    return foo(3);
}

foo(a: int): int{
    return a + 1;
}