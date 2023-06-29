/*
compilationExitCode: 3
*/

foo(): c_void{
    return;
}

main(): i32{
    return foo() + 2;
}