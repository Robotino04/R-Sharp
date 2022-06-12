/*
return: 4
*/

main(): i32 {
    a: i64 = 2;
    b: i64 = 3;
    {
        a: i64 = 1;
        b = b + a;
    }
    return b;
}