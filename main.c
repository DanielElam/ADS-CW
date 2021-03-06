﻿
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

    game->replayIndex = 0;
    movestack_clear(moveStack);

    // loop through every line in the file
    while (fgets(buffer, 255, file)) {
        if (buffer[0] == '#') // header line is ignored
            continue;

        if (buffer[0] == '~') // this line contains the board dimensions
        {
            char* token = strtok(buffer, " ");
            token = strtok(NULL, " ");
            sscanf_s(token, "%d", &boardWidth);
            token = strtok(NULL, " ");
            sscanf_s(token, "%d", &boardHeight);
            continue;
        }

        // otherwise the line is assumed to be a player move.
        char* token = strtok(buffer, " ");

        while (token != NULL)
        {
            token = strtok(NULL, " ");
            if (strcmp(token, "Player") == 0) // found the word "Player", parse the sentence for plr no and column
            {
                token = strtok(NULL, " ");
                sscanf_s(token, "%d", &move.player);

                strtok(NULL, " "); // skip some wording
                strtok(NULL, " ");
                strtok(NULL, " ");

                token = strtok(NULL, " ");
                sscanf_s(token, "%d", &move.column);
                move.column--; // save file columns start with 1, program expects columns starting at 0

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

        // initialize the board with the selected size
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

void show_menu(struct gamestate* game)
{
    char input[1000];

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
        ask_board_size(game);
        game->inMenu = 0;
        game->mode = GAMEMODE_SINGLEPLAYER;
        gamestate_reset(game);
        break;
    case 2:
        ask_board_size(game);
        game->inMenu = 0;
        game->mode = GAMEMODE_TWOPLAYER;
        gamestate_reset(game);
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
            parse_savegame(game, file, &game->replayStack);

            game->inMenu = 0;
            gamestate_reset(game);
            game->mode = GAMEMODE_REPLAY;

            fclose(file);
        }

        break;
    }
}

void game_step(struct gamestate* game)
{
    char input[255];

    if (game->mode == GAMEMODE_TWOPLAYER || game->mode == GAMEMODE_SINGLEPLAYER || game->mode == GAMEMODE_REPLAY)
    {
        const int player = 1 + (game->moveStack.head % 2);
        int winner = gamestate_check_winner(game);
        board_print(&game->board);
        if (winner > 0) {
            if (winner != 3) {
                SetConsoleTextAttribute(consoleHandle, winner == 1 ? COLOUR_PLR1_NOBG : COLOUR_PLR2_NOBG);
                printf("  ! ! ! ! ! ! ! ! ! ! ! ! ! ! !\n");
                printf("  ! ! Player %i won the game ! !\n", winner);
                printf("  ! ! ! ! ! ! ! ! ! ! ! ! ! ! !\n");
            }
            else
            {
                SetConsoleTextAttribute(consoleHandle, COLOUR_DEFAULT);
                printf("  ! ! ! ! ! ! ! ! ! ! ! ! ! ! !\n");
                printf("  ! !   S T A L E M A T E   ! !\n");
                printf("  ! ! ! ! ! ! ! ! ! ! ! ! ! ! !\n");
            }
            SetConsoleTextAttribute(consoleHandle, COLOUR_DEFAULT);
            printf("  > Press enter to return to menu.\n");
            if (game->mode != GAMEMODE_REPLAY)
                gamestate_save(game);
            fgets(input, sizeof input, stdin);
            game->inMenu = 1;
        }
        else
        {
            if (player == 2 && game->mode == GAMEMODE_SINGLEPLAYER) {
                // TODO: AI move
                printf("The AI is making a move...");
                gamestate_push(game, 2, rand() % game->board.width);
                Sleep(1000);
                return;
            }
            else if (game->mode == GAMEMODE_REPLAY) {
                printf("(REPLAY) It's ");
                SetConsoleTextAttribute(consoleHandle, player == 1 ? COLOUR_PLR1_NOBG : COLOUR_PLR2_NOBG);
                printf("player %i's", player);
                SetConsoleTextAttribute(consoleHandle, COLOUR_DEFAULT);
                printf(" turn! (REPLAY)\n");

                // iterate the replayStack
                struct move replayMove = game->replayStack.buffer[game->replayIndex++];
                gamestate_push(game, replayMove.player, replayMove.column);
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
            game->inMenu = 1;
            return;
        }

        if (input[0] == 'z')
        {
            printf("undo...");
            Sleep(10);
            gamestate_undo(game);
            return;
        }

        if (input[0] == 'y')
        {
            printf("redo...");
            Sleep(10);
            gamestate_redo(game);
            return;
        }

        if (input[0] != '\n') {
            int column;
            sscanf_s(input, "%d", &column);

            if (column < 1 || column > game->board.width)
            {
                printf("Bad input...");
                Sleep(2000);
                return;
            }

            gamestate_push(game, player, column - 1);
        }
    }
}

int main(void* args)
{
    int k;
    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    struct gamestate game = {0};
    gamestate_init(&game);
    game.inMenu = 1;

    while (1) {
        clear_console();

        if (game.inMenu == 1)
        {
            show_menu(&game);
        }
        else {
            game_step(&game);
        }
    }

    return 0;
}

