#include <stdio.h>
#include <stdlib.h>
#include "cell.h"
#include "defs.h"

struct cell
{
    struct coordinates coordinates;
    bool status;
    int alive_neighbors;
};

struct cell * create_cell(bool status, struct coordinates coordinates)
{
    struct cell *new_cell;

    new_cell = (struct cell *) malloc(sizeof(struct cell));

    new_cell->status = status;
    new_cell->alive_neighbors = 0;
    new_cell->coordinates = coordinates;

    return new_cell;
}

void set_cell_status(struct cell *cell, bool status)
{
    cell->status = status;
}

bool get_cell_status(struct cell *cell)
{
    return cell->status;
}

void increment_alive_neighbors(struct cell *cell)
{
    cell->alive_neighbors++;
}

int get_alive_neighbors(struct cell *cell)
{
    return cell->alive_neighbors;
}
