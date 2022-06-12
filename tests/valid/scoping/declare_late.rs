/*
return: 3
*/

main(): i32 {
    a: i64 = 2;
    {
        a = 3;
        a: i64 = 0;
    }
    return a;
}