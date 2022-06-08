/*
return: 2
*/

main(): int {
    a: int = 0;
    b: int = 0;
    a ? (b = 1) : (b = 2);
    return b;
}