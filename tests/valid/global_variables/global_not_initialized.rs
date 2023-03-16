/*
executionExitCode: 3
*/

foo: i64;

main(): i32 {
    for (i: i64 = 0; i < 3; i = i + 1)
        foo = foo + 1;
    return foo;
}