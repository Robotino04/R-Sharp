/*
executionExitCode: 6
*/

main(): i32 {
    a: i64 = 0;
    b: i64 = 1;

    while (a < 5) {
        a = a + 2;
        b = b * a;
    }

    return a;
}