/*
fail: 1
*/

foo(): i64 {
    return 3;
}

foo: i64 = 4;

main(): i32 {
    return foo;
}