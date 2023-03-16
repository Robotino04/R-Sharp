/*
executionExitCode: 4
*/

main(): i32 {
    a: i64 = 0;
    for (; ; ) {
        a = a + 1;
        if (a > 3)
            break;
    }

    return a;
}