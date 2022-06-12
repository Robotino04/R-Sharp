/*
return: 0
*/

main(): i32 {
    a: i64 = 1;
    b: i64 = 0;
    a || (b = 5);
    return b;
}