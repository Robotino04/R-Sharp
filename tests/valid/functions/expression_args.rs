/*
return: 14
*/

add(a: i64, b: i64): i64 {
    return a + b;
}

main(): i32 {
    sum: i64 = add(1 + 2, 4);
    return sum + sum;
}
