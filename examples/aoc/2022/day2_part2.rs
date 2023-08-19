putchar, getchar, malloc, memset, free @ std::libc;

print_number @ std::printing;

ROCK: i64 = 0;
PAPER: i64 = 1;
SCISSORS: i64 = 2;
main(): i32 {
    score: i64 = 0;

    while (1){
        character: i32 = getchar();
        if (character == '.'){
            break;
        }
        opponent_choice: i32 = character - 'A';
        getchar(); // space
        
        result: i32 = getchar() - 'X';
        /*
        0: lose
        1: draw
        2: win
        */
        my_choice: i32;
        if (result == 0){
            if (opponent_choice == ROCK){
                my_choice = SCISSORS;
            }
            else if (opponent_choice == PAPER){
                my_choice = ROCK;
            }
            else if (opponent_choice == SCISSORS){
                my_choice = PAPER;
            }
        }
        else if (result == 1){
            my_choice = opponent_choice;
        }
        else if (result == 2){
            if (opponent_choice == ROCK){
                my_choice = PAPER;
            }
            else if (opponent_choice == PAPER){
                my_choice = SCISSORS;
            }
            else if (opponent_choice == SCISSORS){
                my_choice = ROCK;
            }
        }

        score = score + my_choice + 1;
        if (my_choice == opponent_choice){
            score = score + 3;
        }
        else if (my_choice == ROCK && opponent_choice == SCISSORS
            || my_choice == PAPER && opponent_choice == ROCK
            || my_choice == SCISSORS && opponent_choice == PAPER){

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