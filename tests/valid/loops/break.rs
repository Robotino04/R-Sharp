/*
executionExitCode: 15
*/

main(): i32 {
    sum: i64 = 0;
    for (i: i64 = 0; i < 10; i = i + 1) {
        sum = sum + i;
        if (sum > 10)
            break;
    }
    return sum;
}