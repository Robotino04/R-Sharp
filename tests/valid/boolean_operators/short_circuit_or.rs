/*
return: 0
skip: 1
*/

main(): int {
    a: int = 1;
    b: int = 0;
    a || (b = 5);
    return b;
}