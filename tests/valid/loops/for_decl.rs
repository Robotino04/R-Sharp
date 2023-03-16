/*
executionExitCode: 3
*/

main(): i32 {
    a: i64 = 0;

    for (i: i64 = 0; i < 3; i = i + 1)
        a = a + 1;
    return a;
}