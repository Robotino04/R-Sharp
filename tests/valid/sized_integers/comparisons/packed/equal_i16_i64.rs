/*
executionExitCode: 1
*/

main(): i32{
    a: i16 = 5;
    tmp: i64 = -1;
    tmp_p: *c_void = $tmp;
    b: *i64 = tmp_p;
    *b = 5;
    return *b == a;
}