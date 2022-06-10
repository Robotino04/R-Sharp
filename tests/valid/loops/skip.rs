/*
return: 1
*/

main(): int {
    sum: int = 0;
    for (i: int = 0; i < 10; i = i + 1) {
        if ((sum / 2) * 2 != sum)
        skip;
        sum = sum + i;
    }
    return sum;
}