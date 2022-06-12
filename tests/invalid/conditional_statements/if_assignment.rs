/*
fail: 1
*/

main(): i32 {
    flag: i64 = 0;
    a: i64 = if (flag)
                2;
            else
                3;
    return a;
}