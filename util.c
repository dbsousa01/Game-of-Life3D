#include <stdio.h>
#include "defs.h"
#include "util.h"

int mod(int a, int b)
{
    int r = (a < 0) ? (a % b + b) : (a % b);
    return r;
}

void write_output(struct node ****cube, int size)
{
    struct node *aux;
    int x, y;

    for(x = 0; x < size; x++)
    {
        for(y = 0; y < size; y++)
        {
            aux = *(cube[x][y]);
            while(aux != NULL)
            {
                if(aux->status == ALIVE)
                {
                    fprintf(stdout, "%d %d %d\n", aux->x, aux->y, aux->z);
                }
                aux = aux->next;
            }
        }
    }
}