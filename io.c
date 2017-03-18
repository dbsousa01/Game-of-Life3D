#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "io.h"

int read_input_size(FILE *input_fd)
{
    char buffer[BUFFER_SIZE];
    int size;

    fgets(buffer,BUFFER_SIZE,input_fd);
    sscanf(buffer,"%d", &size);

    return size;
}

void * read_input_coordinates(FILE *input_fd, struct coordinates *coordinates)
{
    char buffer[BUFFER_SIZE];

    if(fgets(buffer,BUFFER_SIZE,input_fd) != NULL)
    {
        sscanf(buffer,"%d %d %d", &(coordinates->x), &(coordinates->y), &(coordinates->z));
    }
    else
    {
        return NULL;
    }
}