/*
executionExitCode: 12
*/

foo(a: i64): i64;
bar(b: i64): i64;

main(): i32 {
    return foo(5);
}

foo(a: i64): i64 {
    if (a <= 0) {
        return a;
    }

    return a + bar(a - 1);
}

bar(b: i64): i64 {
    if (b <= 0) {
        return b;
    }

    return b + bar(b / 2);
}