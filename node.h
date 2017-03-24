#ifndef NODE_H
#define NODE_H

struct node
{
    int x, y, z;
    short alive_neighbors;
    short status;
    struct node *next;
};
struct node * add_node(struct node ****cube, int x, int y, int z, short status, int mode);
struct node * create_node(int x, int y, int z, short status);

#endif