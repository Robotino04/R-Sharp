/*
output: "0\n"
*/

putchar @ std::libc;
print_number @ std::printing;

main(): i32 {
    print_number(0);
    putchar('\n');

    return 0;
}