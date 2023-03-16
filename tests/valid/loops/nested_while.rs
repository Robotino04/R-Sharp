/*
executionExitCode: 65
*/

main(): i32 {
    a: i64 = 1;

    while (a / 3 < 20) {
        b: i64 = 1;
        while (b < 10)
            b = b*2;
        a = a + b;
    }

    return a;
}