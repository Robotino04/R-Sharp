/*
return: 1
*/

// This is valid, bacause R# uses the C++ ternary operator.

main(): int {
    a: int = 2;
    b: int = 1;
    a > b ? a = 1 : a = 0;
    return a;
}