#include <stdlib.h>
#include <string.h>
#include "cube.h"
#include "defs.h"
#include "util.h"
//Comments about the functions and their objective are on the corresponding .h

struct node *** create_cube(int size)
{
    struct node ***cube;
    int x, y;

    cube = (struct node ***) malloc(size * sizeof(struct node **));
    for(x = 0; x < size; x++)
    {
        cube[x] = (struct node **) malloc(size * sizeof(struct node *));
        for(y = 0; y < size; y++)
        {
            cube[x][y] = NULL;
        }
    }

    return cube;
}

void destroy_cube(struct node ***cube, int size)
{
    struct node *aux;
    struct node *temp;
    int x, y;

    for(x = 0; x < size; x++)
    {
        for(y = 0; y < size; y++)
        {
            aux = cube[x][y];
            while(aux != NULL)
            {
                temp = aux->next;
                free(aux);
                aux = temp;
            }
        }
        free(cube[x]);
    }
    free(cube);
}

void determine_next_generation(struct node ***cube, int size)
{
    struct node *aux;
    int x, y;

    for(x = 0; x < size; x++)
    {
        for(y = 0; y < size; y++)
        {
            aux = cube[x][y];
            while(aux != NULL)
            {
                if(aux->status == ALIVE)
                {
                    if(aux->alive_neighbors < 2 || aux->alive_neighbors > 4) //conditions to determine if the cell is going to live
                    {
                        aux->status = DEAD;
                    }
                }
                else
                {
                    if(aux->alive_neighbors == 2 || aux->alive_neighbors == 3)
                    {
                        aux->status = ALIVE;
                    }
                }
                aux->alive_neighbors = 0;
                aux = aux->next;
            }
        }
    }
}

void mark_neighbors(struct node ***cube, int size)
{
    struct node *aux;
    int x, y;

    for(x = 0; x < size; x++)
    {
        for(y = 0; y < size; y++)
        {
            aux = cube[x][y];
            while(aux != NULL)
            {
                if(aux->status == ALIVE)
                {   //goes to the neighbours of the cells that are alive and adds a dead node since the alived ones are stored in a counter
                    add_node(&(cube[mod(x+1, size)][y]), aux->z, DEAD, 1);
                    add_node(&(cube[mod(x-1, size)][y]), aux->z, DEAD, 1);
                    add_node(&(cube[x][mod(y+1, size)]), aux->z, DEAD, 1);
                    add_node(&(cube[x][mod(y-1, size)]), aux->z, DEAD, 1);
                    add_node(&(cube[x][y]), mod((aux->z)+1, size), DEAD, 1);
                    add_node(&(cube[x][y]), mod((aux->z)-1, size), DEAD, 1);
                }
                aux = aux->next;
            }
        }
    }
}

void purge(struct node ***cube, int size)
{
    struct node **head;
    struct node *curr;
    struct node *temp;
    int x, y;

    for(x = 0; x < size; x++)
    {
        for(y = 0; y < size; y++)
        {
            head = &(cube[x][y]);
            while((*head) != NULL && (*head)->status == DEAD)
            {
                temp = (*head);
                (*head) = (*head)->next;
                free(temp);
            }
            for(curr = (*head); curr != NULL; curr = curr->next)
            {
                while(curr->next != NULL && curr->next->status == DEAD)
                {
                    temp = curr->next;
                    curr->next = temp->next;
                    free(temp);
                }
            }
        }
    }
}
