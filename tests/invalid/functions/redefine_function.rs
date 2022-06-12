/*
fail: 1
*/

foo(): i64{
    return 3;
}

main(): i32 {
    return foo();
}

foo(): i64{
    return 4;
}