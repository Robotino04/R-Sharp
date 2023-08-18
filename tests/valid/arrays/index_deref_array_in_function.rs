/*
executionExitCode: 33
*/

index_deref_array_in_function(array: *[i64, 5], param2: i32): i64{
    return (*array)[4];
}

main(): i32 {
    numbers: [i64, 5] = [3, 118, 0, -1, 33];
    return index_deref_array_in_function($numbers, 1);
}