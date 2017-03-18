#include <stdio.h>
#include <stdlib.h>
#include "cell.h"
#include "defs.h"
#include "lists.h"

/****************************************************************************/
struct node_list1d
{
    int z;
    struct cell *cell;
    struct node_list1d *next;
    struct node_list1d *prev;
};

void add_cell_list1d(struct list1d **head, struct cell *cell)
{
	struct list1d *new;

	new = (struct list1d *) malloc(sizeof(struct list1d));

    // INITIALIZE NODE VARIABLES
	new->z = cell->coordinates.z;
	new->cell = cell;
    new->next = (*head);
	new->prev = NULL;

	// IF ITS NOT THE FIRST ELEMENT TO BE ADDED THEN ADD IT AT THE START
    if((*head) != NULL)
    {
		(*head)->prev = new;
    }

    // CHANGE LIST HEAD TO NEWLY ADDED NODE
	(*head) = new;
}

void remove_cell_list1d(struct list1d **head, struct list1d *ptr)
{
    // IF THE LIST IS EMPTY OR THERE WASN'T ANYTHING TO BE REMOVED
	if((*head) == NULL || ptr == NULL)
    {
		return;
    }

    // IF THE ITEM TO BE REMOVED IS THE HEAD OF THE LIST
	if((*head) == ptr)
    {
		(*head) = ptr->next;
    }

    // ARRANGE POINTERS OF THE NODES ADJACENT TO THE ONE TO BE REMOVED
	if(ptr->next != NULL)
    {
		ptr->next->prev = ptr->prev;
    }
	if(ptr->prev != NULL)
    {
		ptr->prev->next = ptr->next;
    }

    // FREE THE NODE
	free(ptr);
}

struct list1d * find_cell_list1d(struct list1d *head, struct coordinates coordinates)
{
	struct list1d *aux;

	aux = head;

    // TRAVERSE THE LIST UNTIL THE COORDINATE IS A MATCH AND RETURN THE NODE
	while(aux!= NULL)
	{
		if(aux->x != coordinates.x)
			aux = aux->next;
		else
		{
			return aux;
		}
	}

    // RETURN NULL IF THE NODE WASN'T FOUND
	return NULL;
}
/****************************************************************************/
//********** TEST SUITE **********//
/*
int main(int argc, char *argv[])
{
    struct list1d *head = NULL;
    struct cell *cell = NULL;
    struct list1d *aux = NULL;
    struct coordinates coords0 = {0,0,0};
    struct coordinates coords1 = {1,2,0};
    struct coordinates coords2 = {2,2,0};
    struct coordinates coords3 = {3,2,0};


    // WORKS cell = create_cell(ALIVE, coords1);
    // WORKS add_cell_list1d(&head, cell);
    // WORKS add_cell_list1d(&head, create_cell(ALIVE, coords1));
    // WORKS add_cell_list1d(&head, create_cell(ALIVE, coords2));
    // WORKS add_cell_list1d(&head, create_cell(ALIVE, coords3);
    // WORKS aux = find_cell_list1d(head, coords1);
    // WORKS remove_cell_list1d(&head,find_cell_list1d(head, coords2));
    // WORKS aux = find_cell_list1d(head, coords0);
}
*/