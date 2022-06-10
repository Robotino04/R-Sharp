/*
return: 4
*/

main(): int {
    a: int = 0;
    for (; ; ) {
        a = a + 1;
        if (a > 3)
            break;
    }

    return a;
}