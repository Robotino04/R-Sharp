/*
return: 15
*/

main(): i32 {
    a: i64 = 1 ? 2 ? 3 : 4 : 5;
    b: i64 = 0 ? 2 ? 3 : 4 : 5;
    return a * b;
}