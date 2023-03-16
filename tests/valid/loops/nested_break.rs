/*
executionExitCode: 40
*/

main(): i32 {
    ans: i64 = 0;
    for (i: i64 = 0; i < 5; i = i + 1)
        for (j: i64 = 0; j < 10; j = j + 1)
            if ((i / 2)*2 == i)
                break;
            else
                ans = ans + i;
    return ans;
}