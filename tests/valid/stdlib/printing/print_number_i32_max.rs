/*
output: "2147483647\n"
*/

putchar @ std::libc;
print_number @ std::printing;

main(): i32 {
    print_number(2147483647);
    putchar('\n');

    return 0;
}