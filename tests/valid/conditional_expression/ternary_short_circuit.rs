/*
return: 1
*/

main(): int {
    a: int = 1;
    b: int = 0;
    a ? (b = 1) : (b = 2);
    return b;
}