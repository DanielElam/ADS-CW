#pragma once
#include "str_builder.h"
#include <time.h>
enum gamemode { GAMEMODE_SINGLEPLAYER, GAMEMODE_TWOPLAYER, GAMEMODE_REPLAY };

struct gamestate
{
    struct board board;
    enum gamemode mode;
    struct movestack moveStack;
    struct movestack undoStack;
    struct movestack replayStack;
    int replayIndex;
    int inMenu;
};

/*
    Initialize a gamestate instance
*/
void gamestate_init(struct gamestate* game)
{
    game->mode = (enum gamemode)0;
    movestack_init(&game->moveStack);
    movestack_init(&game->undoStack);
    movestack_init(&game->replayStack);
    game->replayIndex = 0;
}

/*
    Reset counters and clear the board
*/
void gamestate_reset(struct gamestate* game)
{
    movestack_clear(&game->moveStack);
    movestack_clear(&game->undoStack);
    board_clear(&game->board);
}

/*
 * Push a new move onto the move stack
 */
void gamestate_push(struct gamestate* game, const int player, const int column)
{
    struct move move;
    move.player = player;
    move.column = column;

    int y = board_findEmptyRow(&game->board, column);
    if (y == -1) {
        printf("Bad move - column full!");
        return;
    }

    movestack_push(&game->moveStack, move);
    movestack_clear(&game->undoStack);

    *board_getCell(&game->board, column, y) = player;
}

/*
 * Pop the last move from the move stack, push it onto the undo stack
 */
void gamestate_undo(struct gamestate* game)
{
    if (game->moveStack.head == 0)
        return;

    const struct move lastMove = movestack_pop(&game->moveStack);
    movestack_push(&game->undoStack, lastMove);

    struct move undoMove;
    undoMove.player = 0;
    undoMove.column = lastMove.column;

    int y = board_findEmptyRow(&game->board, lastMove.column) + 1;
    *board_getCell(&game->board, lastMove.column, y) = 0;
}

/*
 * Pop the last move from the undo stack, push it onto the move stack
 */
void gamestate_redo(struct gamestate* game)
{
    if (game->undoStack.head == 0)
        return;

    const struct move lastMove = movestack_pop(&game->undoStack);
    movestack_push(&game->moveStack, lastMove);

    int y = board_findEmptyRow(&game->board, lastMove.column);
    *board_getCell(&game->board, lastMove.column, y) = lastMove.player;
}

/*
 * Scan the grid for a row of four pieces placed by a single player
 * Returns: 0 if there is no winner, 1 if player 1 won, 2 if player 2 won, 3 if stalemate (board full)
 */
int gamestate_check_winner(struct gamestate* game)
{
    struct board* board = &game->board;
    int width = board->width;
    int height = board->height;

    int isEmpty = 0;

    // algorithm based on: https://stackoverflow.com/a/38211417
    
    // vertical check 
    for (int j = 0; j < height - 3; j++) {
        for (int i = 0; i < width; i++) {
            for (int player = 1; player <= 2; player++) {
                const int a = *board_getCell(board, i, j);
                const int b = *board_getCell(board, i, j + 1);
                const int c = *board_getCell(board, i, j + 2);
                const int d = *board_getCell(board, i, j + 3);
                if (!a || !b || !c || !d)
                    isEmpty = 1;
                if (a == player && b == player && c == player && d == player) {
                    return player;
                }
            }
        }
    }

    // horizontal check
    for (int i = 0; i < width - 3; i++) {
        for (int j = 0; j < height; j++) {
            for (int player = 1; player <= 2; player++) {
                const int a = *board_getCell(board, i, j);
                const int b = *board_getCell(board, i + 1, j);
                const int c = *board_getCell(board, i + 2, j);
                const int d = *board_getCell(board, i + 3, j);
                if (!a || !b || !c || !d)
                    isEmpty = 1;
                if (a == player && b == player && c == player && d == player) {
                    return player;
                }
            }
        }
    }

    // diagonal check (ascending)
    for (int i = 3; i < width; i++) {
        for (int j = 0; j < height - 3; j++) {
            for (int player = 1; player <= 2; player++) {
                const int a = *board_getCell(board, i, j);
                const int b = *board_getCell(board, i - 1, j + 1);
                const int c = *board_getCell(board, i - 2, j + 2);
                const int d = *board_getCell(board, i - 3, j + 3);
                if (!a || !b || !c || !d)
                    isEmpty = 1;
                if (a == player && b == player && c == player && d == player) {
                    return player;
                }
            }
        }
    }

    // diagonal check (descending)
    for (int i = 3; i < width; i++) {
        for (int j = 3; j < height; j++) {
            for (int player = 1; player <= 2; player++) {
                const int a = *board_getCell(board, i, j);
                const int b = *board_getCell(board, i - 1, j - 1);
                const int c = *board_getCell(board, i - 2, j - 2);
                const int d = *board_getCell(board, i - 3, j - 3);
                if (!a || !b || !c || !d)
                    isEmpty = 1;
                if (a == player && b == player && c == player && d == player) {
                    return player;
                }
            }
        }
    }

    return isEmpty ? 0 : 3;
}

/*
 * Serialize the board size and record of moves to a new file
 */
void gamestate_save(struct gamestate* game)
{
    time_t now = time(NULL);
    struct tm* time = gmtime(&now);

    char filename[40];
    strftime(filename, sizeof(filename), "connect4-save_%Y-%m-%d_%H-%M-%S.txt", time);

    char buffer[255];

    struct str_builder sb;
    str_builder_init(&sb);

    // write the header with board dimensions
    str_builder_addString(&sb, "# Connect 4 Game Save\n", 0);
    sprintf(buffer, "~ %i %i\n", game->board.width, game->board.height);
    str_builder_addString(&sb, buffer, 0);

    // write new line for each move
    int i;
    for (i = 0; i < game->moveStack.head; i++)
    {
        const struct move move = game->moveStack.buffer[i];
        sprintf(buffer, "[Turn %i] Player %i drops into column: %i\n", i + 1, move.player, move.column + 1);
        str_builder_addString(&sb, buffer, 0);
    }

    FILE* file = fopen(filename, "w");

    // exiting program 
    if (file == NULL) {
        printf("Error opening file %s", filename);
        exit(1);
    }
    fprintf(file, "%s", sb.buffer);
    fclose(file);
}