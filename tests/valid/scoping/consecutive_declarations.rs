/*
return: 3
*/

main(): int {
    a: int = 0;
    {
        b: int = 1;
        a = b;
    }
    {
        b: int = 2;
        a = a + b;
    }
    return a;
}