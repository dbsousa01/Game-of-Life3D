#ifndef NODE_H
#define NODE_H

struct node
{
    int z;
    short alive_neighbors;
    short status;
    struct node *next;
};
/**************************************************************************************************************************************
struct node:
It is a Linked List that represents the coordinate z of the cube and it has info about each cell of the cube, the z coordinate,
the number of alive neighbours, its status (dead or alive) and a pointer to the next node on the same z coordinate. Which means that the
program will have several linked lists, each one for the same xy pair of coordinates
**************************************************************************************************************************************/

struct node * add_node(struct node **head, int z, short status, int mode);
/*************************************************************************************************************************************
add_node: 
Adds a new node to the list. 
The function checks if the head is NULL or the already existing head has a higher z coordinate and if 
this happens adds the new node as the new head of the list. The latter is to be able to do an ordered insert on the list, if the z
that we want to insert is lower that the already existing head, this new node should be the head for the sake of order.
If the node is already inserted on the list it is skipped(in case of the head) or the counter of nearby neighbours is incremented.
Else if nothing of this mentioned occurs, the node is simply added to the end of the list.
The function returns a pointer to the newly added node.
*************************************************************************************************************************************/
struct node * create_node(int z, short status);
/************************************************************************************************************************************
create_node:
Creats a new node and fills it up with the available info. The newly created node is returned.
************************************************************************************************************************************/
#endif