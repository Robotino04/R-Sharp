/*
return: 4
*/

foo(b: i64): i64;

main(): i32{
    return foo(3);
}

foo(a: i64): i64{
    return a + 1;
}