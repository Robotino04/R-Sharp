/*
return: 15
*/

main(): int {
    a: int = 1 ? 2 ? 3 : 4 : 5;
    b: int = 0 ? 2 ? 3 : 4 : 5;
    return a * b;
}