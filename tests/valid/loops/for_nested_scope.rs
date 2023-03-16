/*
executionExitCode: 3
*/

main(): i32 {
    i: i64 = 0;
    j: i64 = 0;

    for (i: i64 = 100; i > 0; i = i - 1) {
        i: i64 = 0;
        j: i64 = j * 2 + i;
    }

    k: i64 = 3;

    return j + k;
}