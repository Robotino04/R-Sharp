/*
fail: 1
*/

foo(): int{
    return 3;
}

main(): int {
    return foo();
}

foo(): int{
    return 4;
}