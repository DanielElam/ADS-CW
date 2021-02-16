
#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "console.h"
#include "gamestate.h"


/*
    Prints the board to the console window.
 */
void board_print(struct board* board)
{
    int x, y;

    printf("COL #:");
    for (y = 0; y <= board->height; y++)
    {
        printf(" %i ", y + 1);
    }
    printf("\n");


    for (y = 0; y < board->height; y++)
    {
        printf("      ");
        for (x = 0; x < board->width; x++)
        {
            int plr = *board_getCell(board, x, y);
            int colour = BACKGROUND_BLUE;

            if (plr == 1)
                colour = COLOUR_PLR1;
            else if (plr == 2)
                colour = COLOUR_PLR2;

            SetConsoleTextAttribute(consoleHandle, colour);
            printf(" O ");
        }
        SetConsoleTextAttribute(consoleHandle, COLOUR_DEFAULT);
        printf("\n");
    }
}

int main(void* args)
{
    char input[1000];

    int k;
    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    struct gamestate game;
    gamestate_init(&game);

    while (1) {
        clear_console();
        
        int winner = gamestate_check_winner(&game);

        board_print(&game.board);
        
        const int player = 1 + (game.turns % 2);

        if (game.mode == GAMEMODE_VERSUSPLAYER)
        {
            if (winner > 0) {
                SetConsoleTextAttribute(consoleHandle, winner == 1 ? COLOUR_PLR1_NOBG : COLOUR_PLR2_NOBG);
                printf("Player %i won the game!", winner);
                SetConsoleTextAttribute(consoleHandle, COLOUR_DEFAULT);
                printf(" Press enter to return to menu.\n");
                fgets(input, sizeof input, stdin);
                gamestate_reset(&game);
            }
            else
            {
                printf("It's ");
                SetConsoleTextAttribute(consoleHandle, player == 1 ? COLOUR_PLR1_NOBG : COLOUR_PLR2_NOBG);
                printf("player %i's", player);
                SetConsoleTextAttribute(consoleHandle, COLOUR_DEFAULT);
                printf(" turn! Type a column number, or Z to undo, Y to redo: \n");
                fgets(input, sizeof input, stdin);
            }
            
            if (input[0] == 'z')
            {
                printf("undo...");
                Sleep(10);
                gamestate_undo(&game);
                continue;
            }

            if (input[0] == 'y')
            {
                printf("redo...");
                Sleep(10);
                gamestate_redo(&game);
                continue;
            }

            if (input[0] != '\n') {
                int column;
                sscanf_s(input, "%d", &column);

                if (column < 1 || column > game.board.width)
                {
                    printf("Bad input...");
                    Sleep(2000);
                    continue;
                }

                gamestate_push(&game, player, column - 1);
            }
        }
    }

    printf("Test");
    return 0;
}

