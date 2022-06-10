/*
return: 15
*/

main(): int {
    sum: int = 0;
    for (i: int = 0; i < 10; i = i + 1) {
        sum = sum + i;
        if (sum > 10)
            break;
    }
    return sum;
}