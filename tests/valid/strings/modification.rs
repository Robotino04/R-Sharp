/*
executionExitCode: 0
output: "ABCDEF\n"
*/

putchar @ std::libc;

main(): i32{
    string: [i8, 8] = "ABXDEF\n";
    string[2] = 'C';

    for (i: i32 = 0; string[i] != '\0'; i=i+1){
        putchar(string[i]);
    }
    return 0;
}