/*
return: 30
*/

main(): int {
    sum: int = 0;
    for (i: int = 0; i < 10;) {
        i = i + 1;
        // this is the modulo operator, which is not supported yet
        if (i - (i / 2) * 2)
            skip;
        sum = sum + i;
    }
    return sum;
}