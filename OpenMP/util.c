#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "util.h"
//Comments about the functions and their objective are on the corresponding .h

omp_lock_t ** create_locks(int size)
{
    omp_lock_t **locks;
    int x, y;

    locks = (omp_lock_t **) malloc(size * sizeof(omp_lock_t *));
    for(x = 0; x < size; x++)
    {
        locks[x] = (omp_lock_t *) malloc(size * sizeof(omp_lock_t));
        for(y = 0; y < size; y++)
        {
            omp_init_lock(&(locks[x][y])); //initialization of each cell to an unlocked state
        }
    }
    return locks;
}

int mod(int a, int b)
{
    int r = (a < 0) ? (a % b + b) : (a % b);
    return r;
}

void write_output(struct node ***cube, int size)
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
                    fprintf(stdout, "%d %d %d\n", x, y, aux->z);
                }
                aux = aux->next;
            }
        }
    }
}