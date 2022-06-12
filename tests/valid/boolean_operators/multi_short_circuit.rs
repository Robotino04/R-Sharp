/*
return: 3
skip: 1
*/

main(): int {
    a: int = 0;
    a || (a = 3) || (a = 4);
    return a;
}