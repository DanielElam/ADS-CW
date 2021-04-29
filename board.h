
#pragma once
#include <string.h>

/*
    Stores data about the state of the game board.
*/
struct board
{
    /* The number of columns in the grid */
    int width;
    /* The number of rows in the grid */
    int height;
    /*
        A 3D array of player numbers on the grid. 0 for empty spaces.
        Length is height*width.
    */
    int* grid;
};

/*
    Wipes the board clean.
 */
void board_clear(struct board* board)
{
    memset(board->grid, 0, sizeof(int) * board->width * board->height);
}

/* Initialize the given board struct */
void board_init(struct board* board, int width, int height)
{
    if (board->grid)
        free(board->grid);
    board->width = width;
    board->height = height;
    board->grid = (int*)calloc(width * height, sizeof(int));
}

/* Returns a pointer to the cell in the board's grid */
int* board_getCell(struct board* board, int x, int y)
{
    if (x > board->width || x < 0)
        return NULL;
    if (y > board->height || y < 0)
        return NULL;
    return board->grid + (board->width * y) + x;
}

/*
 * Returns the first empty row from the bottom in the given column, or -1 if there are no empty rows.
 */
int board_findEmptyRow(struct board* board, int column)
{
    int y;
    for (y = board->height - 1; y >= 0; y--)
    {
        int cell = *board_getCell(board, column, y);
        if (cell == 0)
            return y;
    }
    return -1;
}

/*
    Prints the board to the console window.
 */
void board_print(struct board* board)
{
    int x, y;

    printf("COL #:");
    for (y = 0; y < board->width; y++)
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