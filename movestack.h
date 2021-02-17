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


struct movestack
{
    struct move* buffer;
    int head;
    int capacity;
};

void movestack_init(struct movestack* stack)
{
    stack->capacity = 4;
    stack->head = 0;
    stack->buffer = (struct move*)malloc(sizeof(struct move) * stack->capacity);
}

void movestack_clear(struct movestack* movestack)
{
    movestack->head = 0;
}

void movestack_ensure_capacity(struct movestack* stack)
{
    if (stack->head + 1 >= stack->capacity)
    {
        stack->capacity *= 2;
        stack->buffer = (struct move*)realloc(stack->buffer, sizeof(struct move) * stack->capacity);
    }
}

void movestack_push(struct movestack* stack, const struct move move)
{
    movestack_ensure_capacity(stack);
    stack->buffer[stack->head++] = move;
}

struct move movestack_pop(struct movestack* stack)
{
    struct move move;
    move.player = 0;
    move.column = 0;

    if (stack->head != 0)
        move = stack->buffer[--stack->head];

    return move;
}
