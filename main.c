
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
        system("cls");
        board_print(&game.board);

        int player = 1 + (game.turns % 2);

        if (game.mode == GAMEMODE_VERSUSPLAYER)
        {
            printf("It's player %i's turn! Type a column number, or Z to undo, Y to redo: \n", player);
            fgets(input, sizeof input, stdin);


            if (input[0] == 'Z')
            {
                printf("undo...");
                Sleep(2000);
                continue;
            }

            if (input[0] == 'Y')
            {
                printf("redo...");
                Sleep(2000);
                continue;
            }

            int column;
            sscanf_s(input, "%d", &column);

            if (column < 1 || column > game.board.width)
            {
                printf("Bad input...");
                Sleep(2000);
                continue;
            }

            gamestate_push(&game, player, column-1);
        }

        int winner = gamestate_check_winner(&game);

        if (winner > 0)
        {
            printf("Player %d wins!", winner);
            Sleep(2000);
            board_clear(&game.board);
        }
    }

    printf("Test");
    return 0;
}

