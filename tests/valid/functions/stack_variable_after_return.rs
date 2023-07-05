/*
executionExitCode: 0
*/

recurse(depth: i32): c_void{
    if (depth == 0){
        return;
    }
    new_depth: i32 = depth-1;
    recurse(new_depth);
}

main(): i32 {
    recurse(500);

    return 0;
}