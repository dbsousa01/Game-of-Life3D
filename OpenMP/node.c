#include <stdlib.h>
#include "node.h"
//Comments about the functions and their objective are on the corresponding .h

struct node * add_node(struct node **head, int z, short status, int mode)
{
    struct node *new;

    if((*head) == NULL || (*head)->z > z)
    {
        new = create_node(z, status);
        new->alive_neighbors += mode;
        new->next = (*head);
        (*head) = new;
        return new;
    }
    /* SKIP DUPLICATES */
    else if((*head)->z == z)
    {
        (*head)->alive_neighbors += mode;
        return NULL;
    }
    /* INSERT AFTER */
    else
    {
        struct node *aux;
        aux = (*head);
        while((aux->next != NULL) && (aux->next->z <= z))
        {
            aux = aux->next;
        }
        /* EITHER THE NEXT NODE IS NULL OR WE REACHED Z == Z */
        /* IF Z == Z JUST INCREMENT THE COUNT */
        if(aux->z == z)
        {
            aux->alive_neighbors += mode;
            return NULL;
        }
        /* ELSE ADD THE NODE */
        new = create_node(z, status);
        new->alive_neighbors += mode;
        new->next = aux->next;
        aux->next = new;
        return new;
    }
}

struct node * create_node(int z, short status)
{
    struct node *new;

    new = (struct node *) malloc(sizeof(struct node));

    new->z = z;
    new->alive_neighbors = 0;
    new->status = status;
    new->next = NULL;

    return new;
}
