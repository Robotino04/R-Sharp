/*
output: "-12345\n"
*/

putchar @ std::libc;
print_number @ std::printing;

main(): i32 {
    print_number(-12345);
    putchar('\n');

    return 0;
}