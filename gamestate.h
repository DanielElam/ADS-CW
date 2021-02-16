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
    struct move* newMoveStack = malloc(sizeof(struct move) * newCapacity);
    struct move* newUndoStack = malloc(sizeof(struct move) * newCapacity);

    if (game->moveStack != NULL) {
        memcpy(newMoveStack, game->moveStack, sizeof(struct move) * game->moveStackCapacity);
        memcpy(newUndoStack, game->undoStack, sizeof(struct move) * game->moveStackCapacity);
        //free(game->moveStack);
        //free(game->undoStack);
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

/*
 * Push a new move onto the move stack
 */
void gamestate_push(struct gamestate* game, const int player, const int column)
{
    struct move move;
    move.player = player;
    move.column = column;

    const int turn = game->turns++;

    if (turn > game->moveStackCapacity)
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
    gamestate_apply(game, &lastMove);
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
