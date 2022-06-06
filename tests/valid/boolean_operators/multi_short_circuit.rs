/*
return: 3
*/

main(): int {
    a: int = 0;
    a || (a = 3) || (a = 4);
    return a;
}