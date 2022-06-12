/*
return: 8
*/

main(): i32 {
    a: i64 = 0;
    b: i64 = 0;

    if (a)
        a = 2;
    else
        a = 3;

    if (b)
        b = 4;
    else
        b = 5;

    return a + b;
}