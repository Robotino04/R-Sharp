/*
return: 65
*/

main(): int {
    i: int = 0;
    j: int = 0;
    for (i = 0; i < 10; i = i + 1) {
        k: int = i;
        for (i: int = k; i < 10; i = i + 1)
            j = j + 1;
    }
    return j + i;
}