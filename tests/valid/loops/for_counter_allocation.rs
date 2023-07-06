/*
executionExitCode: 10
*/

function(param: i64): i64{
    some_var: i64 = 10;
    // this hung once, because i was not being allocated and always read some_var, which never reached 0.
    for (i: i64 = 5; i != 0; i = i-1){
    }

    return some_var;
}

main(): i32 {
    tmp: i64 = function(666);
    return tmp;
}