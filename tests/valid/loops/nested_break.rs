/*
return: 250
*/

main(): int {
    ans: int = 0;
    for (i: int = 0; i < 10; i = i + 1)
        for (j: int = 0; j < 10; j = j + 1)
            if ((i / 2)*2 == i)
                break;
            else
                ans = ans + i;
    return ans;
}