/*
output: "192\n"
*/

putchar @ std::libc;
print_number @ std::printing;

main(): i32 {
    print_number(192);
    putchar('\n');

    return 0;
}