/*
return: 65
*/

main(): int {
    a: int = 1;

    while (a / 3 < 20) {
        b: int = 1;
        while (b < 10)
            b = b*2;
        a = a + b;
    }

    return a;
}