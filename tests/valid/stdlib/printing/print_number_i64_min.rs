/*
output: "-9223372036854775808\n"
*/

putchar @ std::libc;
print_number @ std::printing;

main(): i32 {
    // R-Sharp doesn't support negative integer literals and this value wouldn't fit into a signed i64
    print_number(-9223372036854775807 - 1);
    putchar('\n');

    return 0;
}