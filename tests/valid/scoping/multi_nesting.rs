/*
executionExitCode: 3
*/

main(): i32 {
    a: i64 = 2;
    if (a < 3) {
        {
            a: i64 = 3;
            return a;
        }
        return a;
    }
}