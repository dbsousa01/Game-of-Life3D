#include <omp.h>
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

	#pragma omp for collapse(2) schedule(guided)
	for(x = 0; x < size; x++)
    {
        for(y = 0; y < size; y++)
        {
            aux = cube[x][y];
            while(aux != NULL)
            {
                if(aux->status == ALIVE)
                {
                    if(aux->alive_neighbors < 2 || aux->alive_neighbors > 4)
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

void mark_neighbors(omp_lock_t **locks, struct node ***cube, int size)
{
    struct node *aux;
    int x, y, z;
    int tmp;

    #pragma omp for collapse(2) schedule(guided) //colapse specifies how many loops in this nested loop should be collapsed
    for(x = 0; x < size; x++)					 //and divided between the threads according to the schedule (guided) 
    {											 //to deal optimally with load balancing
        for(y = 0; y < size; y++)
        {
            aux = cube[x][y];
            while(aux != NULL)
            {
                if(aux->status == ALIVE)
                {
                    z = aux->z;

                    tmp = mod(x+1, size);
                    omp_set_lock(&(locks[tmp][y])); //locks are set so the add is only executed by one thread(eq to serial section)
                    add_node(&(cube[tmp][y]), z, DEAD, 1);// it's useless to have n threads adding the same node...
                    omp_unset_lock(&(locks[tmp][y])); //removes the locks

                    tmp = mod(x-1, size);
                    omp_set_lock(&(locks[tmp][y]));
                    add_node(&(cube[tmp][y]), z, DEAD, 1);
                    omp_unset_lock(&(locks[tmp][y]));

                    tmp = mod(y+1, size);
                    omp_set_lock(&(locks[x][tmp]));
                    add_node(&(cube[x][tmp]), z, DEAD, 1);
                    omp_unset_lock(&(locks[x][tmp]));

                    tmp = mod(y-1, size);
                    omp_set_lock(&(locks[x][tmp]));
                    add_node(&(cube[x][tmp]), z, DEAD, 1);
                    omp_unset_lock(&(locks[x][tmp]));

                    tmp = mod(z+1, size);
                    omp_set_lock(&(locks[x][y]));
                    add_node(&(cube[x][y]), tmp, DEAD, 1);
                    omp_unset_lock(&(locks[x][y]));

                    tmp = mod(z-1, size);
                    omp_set_lock(&(locks[x][y]));
                    add_node(&(cube[x][y]), tmp, DEAD, 1);
                    omp_unset_lock(&(locks[x][y]));
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

    #pragma omp for collapse(2) schedule(guided)//colapse specifies how many loops in this nested loop should be collapsed
    for(x = 0; x < size; x++)                   //and divided between the threads according to the schedule (guided)
    {                                           //to deal optimally with load balancing
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
