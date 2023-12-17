/*
executionExitCode: 1
*/

main(): i32 {
    a: i64 = 1;
    b: i64 = 0;
    if (a)
        b = 1;
    elif (b)
        b = 2;
    return b;
}