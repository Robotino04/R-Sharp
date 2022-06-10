/*
return: 3
*/

main(): int {
    a: int = 0;

    for (i: int = 0; i < 3; i = i + 1)
        a = a + 1;
    return a;
}