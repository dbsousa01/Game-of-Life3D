#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "cube.h"
#include "node.h"
#include "util.h"

int main(int argc, char *argv[])
{
    omp_lock_t **locks; //lock state variable

    FILE *input_fd;

    struct node ***cube;

    char buffer[BUFFER_SIZE];
    char *input_filename;

    int iterations;
    int size;
    int x, y, z;

    if(argc!= 3){
    	printf("Unexpected arguments\n");
    	exit(-1);
    }
    /* ADD ARGUMENT NUMBER AND TYPE CHECKING, ETC */
    input_filename = (char *) malloc((strlen(argv[1]) * sizeof(char)) + 1);
    strcpy(input_filename, argv[1]);
    iterations = atoi(argv[2]);
    if(iterations == 0){ //in case the atoi function got an invalid conversion
    	printf("Error setting the iteration number\n");
    	exit(-1);
    }

    /* OPEN INPUT */
    input_fd = fopen(input_filename, "r");
    if(input_fd ==NULL){ // Unexisting file in directory
    	perror("Error:");
    	exit(-1);
    }
    free(input_filename);

    /* READ SIZE */
    fgets(buffer,BUFFER_SIZE,input_fd);
    sscanf(buffer,"%d", &size);

    /* ALLOC CUBE */
    cube = create_cube(size);
    locks = create_locks(size);

    /* READ INPUT COORDINATES AND ADDS THE NODE ACCORDING TO THE COORDINATES*/
    while(fgets(buffer,BUFFER_SIZE,input_fd) != NULL) 
    {
        sscanf(buffer,"%d %d %d", &x, &y, &z);
        add_node(&(cube[x][y]), z, ALIVE, 0);
    }
    fclose(input_fd); //file closed, no longer needed

    /* PROCESS THE REQUIRED NUMBER OF ITERATIONS */
    #pragma omp parallel 
    {
        while(iterations > 0)
        {
            mark_neighbors(locks, cube, size);
            determine_next_generation(cube, size);
            purge(cube, size);
            #pragma omp single //single makes such that only a thread at a time decrements the variable that follows
            iterations--;
        }
    }

    /* WRITE OUTPUT */
    write_output(cube, size);
    /* FREE MEMORY */
    destroy_cube(cube, size);

    return 1;
}