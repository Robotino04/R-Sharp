/*
output: "Hello, World!\n"
*/

putchar, getchar, memset@ std::libc;

main(): i32 {
    name: [i8, 50];
    memset($name, '_', 50);

    // print "What is your name? "
    {
        question: [i8, 19] = ['W', 'h', 'a', 't', ' ', 'i', 's', ' ', 'y', 'o', 'u', 'r', ' ', 'n', 'a', 'm', 'e', '?', ' '];
        for (i: i32 = 0; i<19; i=i+1){
            putchar(question[i]);
        }
    }
    
    name_length: i32 = 0;
    for (; name_length < 50; name_length = name_length+1){
        name[name_length] = getchar();
        if (name[name_length] == '\n'){   
            break;
        }
    }

    {
        hello: [i8, 6] = ['H', 'e', 'l', 'l', 'o', ' '];
        for (i: i32 = 0; i<6; i=i+1){
            putchar(hello[i]);
        }
    }

    for (i: i32 = 0; i<name_length; i = i+1){
        putchar(name[i]);
    }

    // print "!\n"
    putchar('!');
    putchar('\n');

    return 0;
}