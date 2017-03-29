#ifndef CUBE_H
#define CUBE_H

#include "node.h"

struct node *** create_cube(int size);
void destroy_cube(struct node ***cube, int size);
void determine_next_generation(struct node ***cube, int size);
void mark_neighbors(struct node ***cube, int size);
void purge(struct node ***cube, int size);

#endif