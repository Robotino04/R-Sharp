/*
output: "Hello, World!\n"
*/

putchar, getchar, malloc, memset, free @ std::libc;

main(): i32 {
    name_buffer_length: i32 = 50;
    name: *i32 = malloc(name_buffer_length*4);
    memset(name, '_', name_buffer_length*4);

    // print "What is your name? "

    putchar('W');
    putchar('h');
    putchar('a');
    putchar('t');
    putchar(' ');
    putchar('i');
    putchar('s');
    putchar(' ');
    putchar('y');
    putchar('o');
    putchar('u');
    putchar('r');
    putchar(' ');
    putchar('n');
    putchar('a');
    putchar('m');
    putchar('e');
    putchar('?');
    putchar(' ');
    
    name_length: i32 = 0;
    for (; name_length < name_buffer_length; name_length = name_length+1){
        *(name + name_length) = getchar();
        if (*(name + name_length) == '\n'){   
            break;
        }
    }

    // print "Hello "
    putchar('H');
    putchar('e');
    putchar('l');
    putchar('l');
    putchar('o');
    putchar(' ');

    for (i: i32 = 0; i<name_length; i = i+1){
        putchar(*(name + i));
    }

    // print "!\n"
    putchar('!');
    putchar('\n');

    free(name);

    return 0;
}