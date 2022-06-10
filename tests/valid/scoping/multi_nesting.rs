/*
return: 3
*/

main(): int {
    a: int = 2;
    if (a < 3) {
        {
            a: int = 3;
            return a;
        }
        return a;
    }
}