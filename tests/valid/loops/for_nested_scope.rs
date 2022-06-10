/*
return: 3
*/

main(): int {
    i: int = 0;
    j: int = 0;

    for (i: int = 100; i > 0; i = i - 1) {
        i: int = 0;
        j: int = j * 2 + i;
    }

    k: int = 3;

    return j + k;
}