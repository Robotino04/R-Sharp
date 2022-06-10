/*
return: 6
*/

main(): int {
    a: int = 0;
    b: int = 1;

    while (a < 5) {
        a = a + 2;
        b = b * a;
    }

    return a;
}