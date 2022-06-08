/*
return: 8
*/

main(): int {
    a: int = 0;
    b: int = 0;

    if (a)
        a = 2;
    else
        a = 3;

    if (b)
        b = 4;
    else
        b = 5;

    return a + b;
}