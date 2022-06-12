/*
return: 3
*/

main(): i32 {
    a: i64 = 0;
    if (1)
        if (2)
            a = 3;
        else
            a = 4;

    return a;
}