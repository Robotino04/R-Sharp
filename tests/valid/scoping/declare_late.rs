/*
return: 3
*/

main(): int {
    a: int = 2;
    {
        a = 3;
        a: int = 0;
    }
    return a;
}