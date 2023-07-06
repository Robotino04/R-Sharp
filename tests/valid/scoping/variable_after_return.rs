/*
output: "-9223372036854775808\n"
*/

putchar @ std::libc;

print_number(number: i64): c_void{
    // negative integer limit
    if (number == -9223372036854775807 - 1){
        putchar('-');
        print_number(922337203685477580);
        putchar('8');
        return;
    }
    if (number < 0){
        putchar('-');
        number = -number;
    }
    number_length: i64 = 1;
    number_copy: i64 = number;
    while (number_copy >= 10){
        number_copy = number_copy / 10;
        number_length = number_length * 10;
    }

    for (i: i64 = number_length; i != 0; i = i / 10){
        putchar((number / i) % 10 + '0');
    }
}

main(): i32 {
    // R-Sharp doesn't support negative integer literals and this value wouldn't fit into a signed i64
    print_number(-9223372036854775807 - 1);
    putchar('\n');

    return 0;
}