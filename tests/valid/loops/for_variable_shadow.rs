/*
executionExitCode: 65
*/

main(): i32 {
    i: i64 = 0;
    j: i64 = 0;
    for (i = 0; i < 10; i = i + 1) {
        k: i64 = i;
        for (i: i64 = k; i < 10; i = i + 1)
            j = j + 1;
    }
    return j + i;
}