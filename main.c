
#include <stdio.h>
#include <stdlib.h>
#include "console.h"

#include "movestack.h"
#include "board.h"
#include "gamestate.h"


void parse_savegame(struct gamestate* game, FILE* file, struct movestack* moveStack)
{
    char buffer[255];

    struct move move;

    int boardWidth = 0;
    int boardHeight = 0;

    while (fgets(buffer, 255, file)) {
        if (buffer[0] == '#')
            continue;

        if (buffer[0] == '~')
        {
            char* token = strtok(buffer, " ");
            token = strtok(NULL, " ");
            sscanf_s(token, "%d", &boardWidth);
            token = strtok(NULL, " ");
            sscanf_s(token, "%d", &boardHeight);
            continue;
        }

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

    // load the correct board size
    board_init(&game->board, boardWidth, boardHeight);
}

void ask_board_size(struct gamestate* game)
{
    char input[4];
    int selected = 0;

    while (!selected) {
        clear_console();
        printf("Select a board size\n");
        printf("  1) 7x6\n");
        printf("  2) 5x4\n");
        printf("  3) 6x5\n");
        printf("  4) 8x7\n");
        printf("  5) 9x7\n");
        printf("  6) 10x7\n");
        printf("  7) 8x8\n");

        fgets(input, sizeof input, stdin);
        int sizeSelect;
        sscanf_s(input, "%d", &sizeSelect);

        switch (sizeSelect)
        {
        case 2:
            board_init(&game->board, 5, 4);
            selected = 1;
            break;
        case 3:
            board_init(&game->board, 6, 5);
            selected = 1;
            break;
        case 4:
            board_init(&game->board, 8, 7);
            selected = 1;
            break;
        case 5:
            board_init(&game->board, 9, 7);
            selected = 1;
            break;
        case 6:
            board_init(&game->board, 10, 7);
            selected = 1;
            break;
        case 7:
            board_init(&game->board, 8, 8);
            selected = 1;
            break;
        case 1:
        default:
            board_init(&game->board, 7, 6);
            break;
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

    int replayIndex = 0;
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
                ask_board_size(&game);
                inMenu = 0;
                game.mode = GAMEMODE_SINGLEPLAYER;
                gamestate_reset(&game);
                break;
            case 2:
                ask_board_size(&game);
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
                    replayIndex = 0;
                    parse_savegame(&game, file, &replayStack);

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

                        // iterate the replayStack
                        struct move replayMove = replayStack.buffer[replayIndex++];
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

    return 0;
}

