/*
executionExitCode: 4
*/

a: i64 = 3;

main(): i32 {
    ret: i64 = 0;
    if (a) {
        a: i64 = 0;
        ret = 4;
    }
    return ret;
}