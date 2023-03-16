/*
executionExitCode: 1
*/

main(): i32 {
    sum: i64 = 0;
    for (i: i64 = 0; i < 10; i = i + 1) {
        if ((sum / 2) * 2 != sum)
        skip;
        sum = sum + i;
    }
    return sum;
}