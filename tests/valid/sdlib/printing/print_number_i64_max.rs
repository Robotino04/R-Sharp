/*
output: "9223372036854775807\n"
*/

putchar @ std::libc;
print_number @ std::printing;

main(): i32 {
    print_number(9223372036854775807);
    putchar('\n');

    return 0;
}