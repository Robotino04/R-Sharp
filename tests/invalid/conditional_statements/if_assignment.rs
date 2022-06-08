/*
fail: 1
*/

main(): int {
    flag: int = 0;
    a: int = if (flag)
                2;
            else
                3;
    return a;
}