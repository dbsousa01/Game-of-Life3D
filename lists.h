#ifndef IO_H
#define IO_H

#include "cell.h"
#include "defs.h"

struct node_list1d;
void add_cell_list1d(struct list1d **head, struct cell *cell);
void remove_cell_list1d(struct list1d **head, struct list1d *ptr);
struct list1d * find_cell_list1d(struct list1d *head, struct coordinates coordinates);

#endif