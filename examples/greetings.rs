/*
output: "Hello, World!\n"
*/

putchar, getchar, malloc, memset, free @ std::libc;

main(): i32 {
    name_buffer_length: i32 = 50;
    name: *i32 = malloc(name_buffer_length*4);
    memset(name, 13, name_buffer_length*4);
    name_length: i32 = 0;

    // print "What is your name? "

    putchar(87);
    putchar(104);
    putchar(97);
    putchar(116);
    putchar(32);
    putchar(105);
    putchar(115);
    putchar(32);
    putchar(121);
    putchar(111);
    putchar(117);
    putchar(114);
    putchar(32);
    putchar(110);
    putchar(97);
    putchar(109);
    putchar(101);
    putchar(63);
    putchar(32);

    for (; name_length < name_buffer_length; name_length = name_length+1){
        *(name - -4*name_length) = getchar();
        if (*(name - -4*name_length) == 10){
            break;
        }
    }

    // print "Hello "
    putchar(72);
    putchar(101);
    putchar(108);
    putchar(108);
    putchar(111);
    putchar(32);

    for (i: i32 = 0; i<name_length; i = i+1){
        putchar(*(name - -4*i));
    }

    // print "!\n"
    putchar(33);
    putchar(10);

    free(name);

    return 0;
}