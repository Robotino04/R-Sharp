/*
executionExitCode: 3
*/

main(): i32 {
    a: [i64, 3];
    b: [i64, 3];

    counter: i64 = 0;

    a = b = [counter = counter + 1, counter = counter + 1, counter = counter + 1];
    return counter;
}