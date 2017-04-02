#ifndef CUBE_H
#define CUBE_H

#include "node.h"

struct node *** create_cube(int size);
/*****************************************************************************************************************************************
create_cube:
Takes as parameter the sixe of the cube and initiates a matrix with the intended size. A pointer to the newly created matrix
is returned.
*****************************************************************************************************************************************/
void destroy_cube(struct node ***cube, int size);
/****************************************************************************************************************************************
destroy_cube:
Transverses through the cube and frees every memory poisition of it. Good memory management.
****************************************************************************************************************************************/
void determine_next_generation(struct node ***cube, int size);
/***************************************************************************************************************************************
determine_next_generation:
Transverses through the cube and applies the living/dead conditions based on the number
of alive neighbours of each cell and "tags" the status of the cell to the next iteration (if the cell is alive or dead).
At each loop end it resets the amount of alive neighbours of each cell so mix ups between generations are avoided.
***************************************************************************************************************************************/
void mark_neighbors(struct node ***cube, int size);
/****************************************************************************************************************************************
mark_neighbours:
Goes through the cube, selects the alive cells and marks each of its neighbours as dead which means they were already processed.
****************************************************************************************************************************************/
void purge(struct node ***cube, int size);
/****************************************************************************************************************************************
purge:
Transverses through the cube and frees all cells that are dead, since they are no longer needed. The important ones for the next 
generation are the ones that are alive.
****************************************************************************************************************************************/

#endif