#ifndef CELL_H
#define CELL_H

struct cell;

struct cell * create_cell(bool status, struct coordinates coordinates);
void set_cell_status(struct cell *cell, bool status);
bool get_cell_status(struct cell *cell);
void increment_alive_neighbors(struct cell *cell);
int get_alive_neighbors(struct cell *cell);

#endif