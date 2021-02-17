
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "movestack.h"
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

void parse_savegame(struct gamestate* game, FILE* file, int* replay_turn_count, struct movestack* moveStack)
{
    char buffer[255];

    struct move move;

    while (fgets(buffer, 255, file)) {
        printf("%s\n", buffer);

        if (buffer[0] == '#')
            continue;

        char* token = strtok(buffer, " ");

        while (token != NULL)
        {
            token = strtok(NULL, " ");
            if (strcmp(token, "Player") == 0)
            {
                token = strtok(NULL, " ");
                sscanf_s(token, "%d", &move.player);

                strtok(NULL, " ");
                strtok(NULL, " ");
                strtok(NULL, " ");
                token = strtok(NULL, " ");
                sscanf_s(token, "%d", &move.column);
                move.column--;

                movestack_push(moveStack, move);
                break;
            }
        }
    }
}

int main(void* args)
{
    char input[1000];

    int inMenu = 1;

    int k;
    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    struct gamestate game;
    gamestate_init(&game);

    int replayTurnCount;
    struct movestack replayStack;
    movestack_init(&replayStack);

    while (1) {
        clear_console();

        if (inMenu == 1)
        {
            printf(" \n");
            printf(" | Connect Four\n");
            printf(" | Written by Daniel Elam\n");
            printf(" \n");
            printf("  1) Singleplayer (AI)\n");
            printf("  2) Two Player\n");
            printf("  3) Watch Replay\n");

            fgets(input, sizeof input, stdin);
            int select;
            sscanf_s(input, "%d", &select);

            switch (select)
            {
            case 1:
                inMenu = 0;
                game.mode = GAMEMODE_SINGLEPLAYER;
                gamestate_reset(&game);
                break;
            case 2:
                inMenu = 0;
                game.mode = GAMEMODE_TWOPLAYER;
                gamestate_reset(&game);
                break;
            case 3:
                FILE * file = NULL;
                while (1) {
                    clear_console();
                    printf("Enter the path to the save file: ");
                    fgets(input, sizeof input, stdin);
                    strtok(input, "\n");

                    if (input[0] == '\n')
                        break;

                    printf("\n");
                    file = fopen(input, "r");
                    if (file == NULL) {
                        printf("Could not open file %s", input);
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }

                if (file != NULL) {
                    parse_savegame(&game, file, &replayTurnCount, &replayStack);

                    inMenu = 0;
                    gamestate_reset(&game);
                    game.mode = GAMEMODE_REPLAY;

                    fclose(file);
                }

                break;
            }
        }
        else {
            if (game.mode == GAMEMODE_TWOPLAYER || game.mode == GAMEMODE_SINGLEPLAYER || game.mode == GAMEMODE_REPLAY)
            {
                const int player = 1 + (game.moveStack.head % 2);
                int winner = gamestate_check_winner(&game);
                board_print(&game.board);
                if (winner > 0) {
                    SetConsoleTextAttribute(consoleHandle, winner == 1 ? COLOUR_PLR1_NOBG : COLOUR_PLR2_NOBG);
                    printf("  ! ! ! ! ! ! ! ! ! ! ! ! ! ! !\n");
                    printf("  ! ! Player %i won the game ! !\n", winner);
                    printf("  ! ! ! ! ! ! ! ! ! ! ! ! ! ! !\n");
                    SetConsoleTextAttribute(consoleHandle, COLOUR_DEFAULT);
                    printf("  > Press enter to return to menu.\n");
                    if (game.mode != GAMEMODE_REPLAY)
                        gamestate_save(&game);
                    fgets(input, sizeof input, stdin);
                    inMenu = 1;
                }
                else
                {
                    if (player == 2 && game.mode == GAMEMODE_SINGLEPLAYER) {
                        // TODO: AI move
                        printf("The AI is making a move...");
                        gamestate_push(&game, 2, rand() % game.board.width);
                        Sleep(1000);
                        continue;
                    }
                    else if (game.mode == GAMEMODE_REPLAY) {
                        printf("(REPLAY) It's ");
                        SetConsoleTextAttribute(consoleHandle, player == 1 ? COLOUR_PLR1_NOBG : COLOUR_PLR2_NOBG);
                        printf("player %i's", player);
                        SetConsoleTextAttribute(consoleHandle, COLOUR_DEFAULT);
                        printf(" turn! (REPLAY)\n");

                        struct move replayMove = movestack_pop(&replayStack);
                        gamestate_push(&game, replayMove.player, replayMove.column);
                        input[0] = '\n';

                        Sleep(1000);
                    }
                    else {
                        printf("It's ");
                        SetConsoleTextAttribute(consoleHandle, player == 1 ? COLOUR_PLR1_NOBG : COLOUR_PLR2_NOBG);
                        printf("player %i's", player);
                        SetConsoleTextAttribute(consoleHandle, COLOUR_DEFAULT);
                        printf(" turn! Type a column number, or Z to undo, Y to redo, Q to quit: \n");
                        fgets(input, sizeof input, stdin);
                    }
                }

                if (input[0] == 'q')
                {
                    inMenu = 1;
                    continue;
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
    }

    printf("Test");
    return 0;
}

