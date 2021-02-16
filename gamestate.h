#pragma once

/*
    Stores data about a player's move
*/
struct move
{
    /*
        The player that made the move
    */
    int player;
    /*
        The column the player selected
    */
    int column;
};

enum gamemode { GAMEMODE_VERSUSPLAYER, GAMEMODE_VERSUSAI };

struct gamestate
{
    struct board board;
    enum gamemode mode;
    int turns;
    int undoCount;
    struct move* moveStack;
    struct move* undoStack;
    int moveStackCapacity;
};

void gamestate_apply(struct gamestate* game, const struct move* move)
{
    int y = board_findEmptyRow(&game->board, move->column);
    int* cell = board_getCell(&game->board, move->column, y);
    *cell = move->player;
}

/*
    Resize the move and undo stacks to the given capacity.
 */
void gamestate_resize(struct gamestate* game, int newCapacity)
{
    int test = sizeof(struct move);

    struct move* newMoveStack = calloc(newCapacity, sizeof(struct move));
    struct move* newUndoStack = calloc(newCapacity, sizeof(struct move));

    if (game->moveStack != NULL) {
        memcpy(newMoveStack, game->moveStack, sizeof(struct move) * game->moveStackCapacity);
        memcpy(newUndoStack, game->undoStack, sizeof(struct move) * game->moveStackCapacity);
        free(game->moveStack);
        free(game->undoStack);
    }

    game->moveStack = newMoveStack;
    game->undoStack = newUndoStack;

    game->moveStackCapacity = newCapacity;
}

void gamestate_init(struct gamestate* game)
{
    board_init(&game->board, 7, 6);
    game->mode = 0;
    game->turns = 0;
    game->undoCount = 0;
    game->moveStackCapacity = 2;
    game->moveStack = NULL;
    game->undoStack = NULL;
    gamestate_resize(game, game->moveStackCapacity);
}

void gamestate_reset(struct gamestate* game)
{
    game->turns = 0;
    game->undoCount = 0;
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

    const int turn = game->turns++;

    if (turn > game->moveStackCapacity-1)
    {
        // we've hit the limit, double the size of the stack
        const int newCapacity = game->moveStackCapacity * 2;
        gamestate_resize(game, newCapacity);
    }

    game->moveStack[turn] = move;
    game->undoCount = 0;
    gamestate_apply(game, &move);
}

/*
 * Pop a new move from the move stack, push it onto the undo stack
 */
void gamestate_undo(struct gamestate* game)
{
    if (game->turns == 0)
        return;

    const struct move lastMove = game->moveStack[game->turns-1];
    game->undoStack[game->undoCount] = lastMove;
    game->turns--;
    game->undoCount++;

    struct move undoMove;
    undoMove.player = 0;
    undoMove.column = lastMove.column;

    int y = board_findEmptyRow(&game->board, lastMove.column) + 1;
    int* cell = board_getCell(&game->board, lastMove.column, y);
    *cell = 0;
}

/*
 * Pop a new move from the undo stack, push it onto the move stack
 */
void gamestate_redo(struct gamestate* game)
{
    if (game->undoCount == 0)
        return;

    const struct move lastMove = game->undoStack[game->undoCount - 1];
    game->moveStack[game->turns] = lastMove;
    game->turns++;
    game->undoCount--;
    gamestate_apply(game, &lastMove);
}


// scan the grid for a a row of fours
// algorithm based on https://stackoverflow.com/a/38211417
int gamestate_check_winner_scan(struct gamestate* game, int player)
{
    struct board* board = &game->board;
    int width = board->width;
    int height = board->height;

    // horizontal check 
    for (int j = 0; j < height - 3; j++) {
        for (int i = 0; i < width; i++) {
            if (*board_getCell(board, i, j) == player && *board_getCell(board, i, j+1) == player && *board_getCell(board, i, j+2) == player && *board_getCell(board, i, j+3) == player) {
                return 1;
            }
        }
    }
    // vertical check
    for (int i = 0; i < width - 3; i++) {
        for (int j = 0; j < height; j++) {
            if (*board_getCell(board, i, j) == player && *board_getCell(board, i+1, j) == player && *board_getCell(board, i + 2, j) == player && *board_getCell(board, i+3, j) == player) {
                return 1;
            }
        }
    }
    // diagonal check (ascending)
    for (int i = 3; i < width; i++) {
        for (int j = 0; j < height - 3; j++) {
            if (*board_getCell(board, i, j) == player && *board_getCell(board, i - 1, j + 1) == player && *board_getCell(board, i - 2, j + 2) == player && *board_getCell(board, i - 3, j + 3) == player)
                return 1;
        }
    }
    // diagonal check (descending)
    for (int i = 3; i < width; i++) {
        for (int j = 3; j < height; j++) {
            if (*board_getCell(board, i, j) && *board_getCell(board, i - 1, j - 1) == player && *board_getCell(board, i - 2, j - 2) == player && *board_getCell(board, i - 3, j - 3) == player)
                return 1;
        }
    }
    return 0;
}

int gamestate_check_winner(struct gamestate* game)
{
    if (gamestate_check_winner_scan(game, 1) == 1)
        return 1;
    else if (gamestate_check_winner_scan(game, 2) == 1)
        return 2;
    return 0;
}