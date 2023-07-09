putchar, getchar, malloc, memset, free @ std::libc;

print_number @ std::printing;

read_number(failed: *i8): i64{
    number: i64 = 0;
    character: i32 = getchar();
    if (character == '.'){
        *failed = 2;
        return 0;
    }
    if (character < '0' || character > '9'){
        *failed = 1;
        return 0;
    }

    do{
        if (character == '.'){
            *failed = 2;
            return 0;
        }
        if (character < '0' || character > '9'){
            *failed = 1;
            return 0;
        }
        number = number * 10 + (character - '0');
    }
    while ((character = getchar()) != '\n');
    return number;
}

max_index(array: *i64, length: i32): i32{
    current_max: i64 = *array;
    current_max_index: i32 = 0;

    for (i: i32 = 0; i < length; i = i+1){
        if (*(array + i) > current_max){
            current_max = *(array + i);
            current_max_index = i;
        }
    }
    return current_max_index;
}

main(): i32 {
    MAX_NUMBER_ELVES: i32 = 200;
    calories: *i64 = malloc(MAX_NUMBER_ELVES * 8);
    memset(calories, 0, MAX_NUMBER_ELVES * 8);
    
    calorie_index: i32 = 0;
    for (;calorie_index < MAX_NUMBER_ELVES; calorie_index = calorie_index + 1){
        failed: i8 = 0;
        while (!failed){
            *(calories + calorie_index) = *(calories + calorie_index) + read_number($failed);
        }
        if (failed == 2){
            break;
        }
    }

    // part 1

    putchar('1');
    putchar('|');
    print_number(*(calories + max_index(calories, calorie_index)));
    putchar('\n');

    // part 2

    top_three_calories: i64 = 0;
    for (i: i32 = 0; i<3; i = i+1){
        the_max_index: i32 = max_index(calories, calorie_index);
        top_three_calories = top_three_calories + *(calories + the_max_index);
        *(calories + the_max_index) = 0;
    }
    putchar('3');
    putchar('|');
    print_number(top_three_calories);
    putchar('\n');

    return 0;
}