/*
return: -1
*/

foo(a: i64, b: i64): i64;

main(): i32 {
    return foo(1, 2);
}

foo(x: i64, y: i64): i64{
    return x - y;
}