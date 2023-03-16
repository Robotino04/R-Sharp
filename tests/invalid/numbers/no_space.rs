/*
compilationExitCode: 3
*/

// this will create a semantic error because "return0" is treated like an identifier (that is nowhere defined)

main(): i32 {
    return0;
}