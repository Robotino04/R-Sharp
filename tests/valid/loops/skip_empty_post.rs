/*
executionExitCode: 30
*/

main(): i32 {
    sum: i64 = 0;
    for (i: i64 = 0; i < 10;) {
        i = i + 1;
        // this is the modulo operator, which is not supported yet
        if (i - (i / 2) * 2)
            skip;
        sum = sum + i;
    }
    return sum;
}