/*
return: 4
*/

main(): int {
    a: int = 0;
    if (a) {
        b: int = 2;
        return b;
    } else {
        c: int = 3;
        if (a < c) {
            return 4;
        } else {
            return 5;
        }
    }
    return a;
}