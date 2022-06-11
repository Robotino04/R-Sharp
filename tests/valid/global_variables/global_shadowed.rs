/*
return: 4
*/

a: int = 3;

main(): int {
    ret: int = 0;
    if (a) {
        a: int = 0;
        ret = 4;
    }
    return ret;
}