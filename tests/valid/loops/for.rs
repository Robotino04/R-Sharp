/*
return: 3
*/

main(): i32 {
    a: i64 = 0;

    for (a = 0; a < 3; a = a + 1)
        a = a * 2;
    return a;
}