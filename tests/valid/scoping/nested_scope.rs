/*
return: 4
*/

main(): int {
    a: int = 2;
    b: int = 3;
    {
        a: int = 1;
        b = b + a;
    }
    return b;
}