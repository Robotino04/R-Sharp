/*
executionExitCode: 0
output: "Hello, World!\n"
*/

putchar @ std::libc;

main(): i32{
    string: [i8, 15] = "Hello, World!\n";
    for (i: i32 = 0; string[i] != '\0'; i=i+1){
        putchar(string[i]);
    }
    return 0;
}