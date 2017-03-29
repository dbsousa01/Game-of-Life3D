#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "cube.h"
#include "node.h"
#include "util.h"

int main(int argc, char *argv[])
{
    FILE *input_fd;

    struct node ***cube;

    char buffer[BUFFER_SIZE];
    char *input_filename;

    int iterations;
    int size;
    int x, y, z;

    /* ADD ARGUMENT NUMBER AND TYPE CHECKING, ETC */
    input_filename = (char *) malloc((strlen(argv[1]) * sizeof(char)) + 1);
    strcpy(input_filename, argv[1]);
    iterations = atoi(argv[2]);

    /* OPEN INPUT */
    input_fd = fopen(input_filename, "r");
    free(input_filename);

    /* READ SIZE */
    fgets(buffer,BUFFER_SIZE,input_fd);
    sscanf(buffer,"%d", &size);

    /* ALLOC CUBE */
    cube = create_cube(size);

    /* READ INPUT COORDINATES */
    while(fgets(buffer,BUFFER_SIZE,input_fd) != NULL)
    {
        sscanf(buffer,"%d %d %d", &x, &y, &z);
        add_node(&(cube[x][y]), z, ALIVE, 0);
    }
    fclose(input_fd);

    /* PROCESS THE REQUIRED NUMBER OF ITERATIONS */
    while(iterations > 0)
    {
        mark_neighbors(cube, size);
        determine_next_generation(cube, size);
        purge(cube, size);
        iterations--;
    }

    /* WRITE OUTPUT */
    write_output(cube, size);
    /* FREE MEMORY */
    destroy_cube(cube, size);

    return 1;
}