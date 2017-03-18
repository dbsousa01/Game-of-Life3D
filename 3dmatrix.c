#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cell.h"
#include "defs.h"
#include "io.h"

/****************************************************************************/
void add_cell_generation3d(struct cell ****generation, struct cell *cell)
{
    int x, y, z;
    x = cell->coordinates.x;
    y = cell->coordinates.y;
    z = cell->coordinates.z;
    free((generation[x][y][z]));
    generation[x][y][z] = cell;
}

struct cell **** create_generation3d(int size)
{
    struct cell ****new;
    struct coordinates temp;

    new = (struct cell ****) malloc(size * sizeof(struct cell ***));

    for(temp.x = 0; temp.x < size; temp.x++)
    {
        new[temp.x] = (struct cell ***) malloc(size * sizeof(struct cell **));
        for(temp.y = 0; temp.y < size; temp.y++)
        {
            new[temp.x][temp.y] = (struct cell **) malloc(size * sizeof(struct cell *));
            for(temp.z = 0; temp.z < size; temp.z++)
            {
                new[temp.x][temp.y][temp.z] = create_cell(DEAD, temp);
            }
        }
    }
    return new;
}
void destroy_generation3d(struct cell ****generation, int size)
{
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            for(int k = 0; k < size; k++)
            {
                free(generation[i][j][k]);
            }
            free(generation[i][j]);
        }
        free(generation[i]);
    }
    free(generation);
}
void increase_neighbor_count3d(struct cell ****generation, struct cell *cell, int size)
{
    int x = cell->coordinates.x;
    int y = cell->coordinates.y;
    int z = cell->coordinates.z;

    if(x + 1 < size)
        generation[x + 1][y][z]->alive_neighbors++;
    else
        generation[0][y][z]->alive_neighbors++;
    if(x - 1 >= 0)
        generation[x - 1][y][z]->alive_neighbors++;
    else
        generation[size - 1][y][z]->alive_neighbors++;
    if(y + 1 < size)
        generation[x][y + 1][z]->alive_neighbors++;
    else
        generation[x][0][z]->alive_neighbors++;
    if(y - 1 >= 0)
        generation[x][y - 1][z]->alive_neighbors++;
    else
        generation[x][size - 1][z]->alive_neighbors++;
    if(z + 1 < size)
        generation[x][y][z + 1]->alive_neighbors++;
    else
        generation[x][y][0]->alive_neighbors++;
    if(z - 1 >= 0)
        generation[x][y][z - 1]->alive_neighbors++;
    else
        generation[x][y][size - 1]->alive_neighbors++;
}

void print_generation3d(struct cell ****generation, int size)
{
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            printf("|");
            for(int k = 0; k < size; k++)
            {
                printf("%d|", generation[i][j][k]->status);
            }
            printf("\n");
        }
        printf("-------\n");
    }
}
void process_current_generation3d(struct cell ****current_generation, struct cell ****next_generation, int size)
{
    struct coordinates temp;

    for(temp.x = 0; temp.x < size; temp.x++)
    {
        for(temp.y = 0; temp.y < size; temp.y++)
        {
            for(temp.z = 0; temp.z < size; temp.z++)
            {
                if(current_generation[temp.x][temp.y][temp.z]->status == ALIVE)
                {
                    increase_neighbor_count3d(current_generation, current_generation[temp.x][temp.y][temp.z], size);
                }
            }
        }
    }
    for(temp.x = 0; temp.x < size; temp.x++)
    {
        for(temp.y = 0; temp.y < size; temp.y++)
        {
            for(temp.z = 0; temp.z < size; temp.z++)
            {
                if((current_generation[temp.x][temp.y][temp.z]->status == ALIVE))
                {
                    if(current_generation[temp.x][temp.y][temp.z]->alive_neighbors >= 2 && current_generation[temp.x][temp.y][temp.z]->alive_neighbors <= 4)
                    {
                        add_cell_generation3d(next_generation, create_cell(ALIVE, temp));
                    }
                }
                else
                {
                    if(current_generation[temp.x][temp.y][temp.z]->alive_neighbors >= 2 && current_generation[temp.x][temp.y][temp.z]->alive_neighbors <= 3)
                    {
                        add_cell_generation3d(next_generation, create_cell(ALIVE, temp));
                    }
                }
                current_generation[temp.x][temp.y][temp.z]->status = DEAD;
                current_generation[temp.x][temp.y][temp.z]->alive_neighbors = 0;
            }
        }
    }
}
/****************************************************************************/
int main(int argc, char *argv[])
{
    FILE *input_fd;
    struct cell ****aux;
    struct cell ****current_generation;
    struct cell ****next_generation;
    struct coordinates coordinates = { .x = -1, .y = -1, .z = -1 };
    char *input_filename;
    int iterations;
    int size;
    int i, j, k, l;

    /* ADD ARGUMENT NUMBER AND TYPE CHECKING, ETC */
    input_filename = (char *) malloc((strlen(argv[1]) * sizeof(char)) + 1);
    strcpy(input_filename, argv[1]);
    iterations = atoi(argv[2]);

    input_fd = fopen(input_filename, "r");
    if (input_fd == NULL)
    {
        printf("Error opening input file\n");
        exit(99);
    }
    size = read_input_size(input_fd);

    current_generation = create_generation3d(size);
    next_generation = create_generation3d(size);
    while(read_input_coordinates(input_fd, &coordinates) != NULL)
    {
        add_cell_generation3d(current_generation, create_cell(ALIVE, coordinates));
    }
    printf("Original\n");
    print_generation3d(current_generation, size);
    for(i = 0; i < iterations; i++)
    {
        process_current_generation3d(current_generation, next_generation, size);
        aux = next_generation;
        next_generation = current_generation;
        current_generation = aux;
        printf("Iteration %d\n", i + 1);
        print_generation3d(current_generation, size);
    }
    destroy_generation3d(current_generation,size);
    destroy_generation3d(next_generation,size);

    free(input_filename);
    fclose(input_fd);
}
