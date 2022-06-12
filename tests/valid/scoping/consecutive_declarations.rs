/*
return: 3
*/

main(): i32 {
    a: i64 = 0;
    {
        b: i64 = 1;
        a = b;
    }
    {
        b: i64 = 2;
        a = a + b;
    }
    return a;
}