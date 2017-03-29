#ifndef NODE_H
#define NODE_H

struct node
{
    int z;
    short alive_neighbors;
    short status;
    struct node *next;
};

struct node * add_node(struct node **head, int z, short status, int mode);
struct node * create_node(int z, short status);

#endif