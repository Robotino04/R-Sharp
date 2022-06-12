/*
return: 4
*/

main(): i32 {
    a: i64 = 0;
    if (a) {
        b: i64 = 2;
        return b;
    } else {
        c: i64 = 3;
        if (a < c) {
            return 4;
        } else {
            return 5;
        }
    }
    return a;
}