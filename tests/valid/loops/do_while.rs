/*
executionExitCode: 16
*/

main(): i32 {
    a: i64 = 1;
    do {
        a = a * 2;
    } while(a < 11);

    return a;
}