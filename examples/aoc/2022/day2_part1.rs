putchar, getchar, malloc, memset, free @ std::libc;

print_number @ std::printing;

main(): i32 {
    score: i64 = 0;

    while (1){
        character: i32 = getchar();
        if (character == '.'){
            break;
        }
        opponent_choice: i32 = character - 'A';
        getchar(); // space
        my_choice: i32 = getchar() - 'X';

        /*
        0: Rock (wins against 2)
        1: Paper (wins against 0)
        2: Scissors (wins against 1)
        */

        score = score + my_choice + 1;
        if (my_choice == opponent_choice){
            score = score + 3;
        }
        else if (my_choice == 0 && opponent_choice == 2
            || my_choice == 1 && opponent_choice == 0
            || my_choice == 2 && opponent_choice == 1){

            // win
            score = score + 6;
        }
        else{
            // loss
        }
        
        // newline
        getchar();
    }

    print_number(score);
    putchar('\n');

    return 0;
}