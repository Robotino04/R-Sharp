/*
executionExitCode: 2
*/

main(): i32 {
    a: i64 = 0;
    b: i64 = 1;
    if (a)
        b = 1;
    elif (b)
        b = 2;
    return b;
}