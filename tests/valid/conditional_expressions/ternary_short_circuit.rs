/*
return: 1
*/

main(): i32 {
    a: i64 = 1;
    b: i64 = 0;
    a ? (b = 1) : (b = 2);
    return b;
}