/*
executionExitCode: 19
*/

main(): i32 {
    a: i64 = 4;
    b: i64 = 17;

    ptr: *i64 = $a;
    *ptr = 2;
    ptr = $b;
    return a + *ptr;
}
