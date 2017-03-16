
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct coordinates
{
    int x;
    int y;
    int z;
};
struct cell
{
    struct coordinates coordinates;
    bool alive;
    int alive_neighbors;
};
struct coordinates get_cell_coordinates(struct cell cell)
{
    return cell.coordinates;
}
int get_coordinate_x(struct coordinates coordinates)
{
    return coordinates.x;
}
int get_coordinate_y(struct coordinates coordinates)
{
    return coordinates.y;
}
int get_coordinate_z(struct coordinates coordinates)
{
    return coordinates.z;
}
void print_cell_coordinates(struct cell cell)
{
    struct coordinates temp;
    int x, y, z;

    temp = get_cell_coordinates(cell);
    x = get_coordinate_x(temp);
    y = get_coordinate_y(temp);
    z = get_coordinate_z(temp);

    printf("(%d,%d,%d) ", x, y, z);
}
void print_cell_neighbors(struct cell cell)
{
    printf("%d",cell.alive_neighbors);
}
void print_cell_status(struct cell cell)
{
    printf("%d ",cell.alive);
}
void print_generation_alive_status(struct cell ***generation, int size)
{
    int i, j, k;
    /* PRINT */
    for(i = 0; i < size; i++)
    {
        printf("\nz = %d\n", i);
        for(j = 0; j < size; j++)
        {
            for(k = 0; k < size; k++)
            {
                print_cell_status(generation[i][j][k]);
            }
            printf("\n");
        }
    }
}
void print_generation_alive_neighbors(struct cell ***generation, int size)
{
    int i, j, k;
    /* PRINT */
    for(i = 0; i < size; i++)
    {
        printf("\nz = %d\n", i);
        for(j = 0; j < size; j++)
        {
            for(k = 0; k < size; k++)
            {
                print_cell_neighbors(generation[i][j][k]);
            }
            printf("\n");
        }
    }
}


/*
void print_dimension_x(struct cell)
{

}
void print_dimension_y(struct cell)
{

}
void print_dimension_z(struct cell)
{

}
*/





int main(int argc, char *argv[])
{
	char *input_filename;
	int size;

	struct cell ***aux;
	struct cell ***current_generation;
	struct cell ***next_generation;
	int i, j, k, l;



	FILE *input_fd;


	input_filename = (char *) malloc(strlen(argv[1]) * sizeof(char));
	size = atoi(argv[2]);

	/* input_fd = fopen(input_filename, "r"); */

	current_generation = (struct cell ***) malloc(size * sizeof(struct cell **));
	for(i = 0; i < size; i++)
	{
		current_generation[i] = (struct cell **) malloc(size * sizeof(struct cell *));
		for(j = 0; j < size; j++)
		{
			current_generation[i][j] = (struct cell *) malloc(size * sizeof(struct cell));
			for(k = 0; k < size; k++)
			{
				current_generation[i][j][k].alive = 0;
				current_generation[i][j][k].alive_neighbors = 0;
				current_generation[i][j][k].coordinates.x = k;
				current_generation[i][j][k].coordinates.y = j;
				current_generation[i][j][k].coordinates.z = i;
			}
		}
	}
	next_generation = (struct cell ***) malloc(size * sizeof(struct cell **));
	for(i = 0; i < size; i++)
	{
		next_generation[i] = (struct cell **) malloc(size * sizeof(struct cell *));
		for(j = 0; j < size; j++)
		{
			next_generation[i][j] = (struct cell *) malloc(size * sizeof(struct cell));
			for(k = 0; k < size; k++)
			{
				next_generation[i][j][k].alive = 0;
				next_generation[i][j][k].alive_neighbors = 0;
				next_generation[i][j][k].coordinates.x = k;
				next_generation[i][j][k].coordinates.y = j;
				next_generation[i][j][k].coordinates.z = i;
			}
		}
	}

	current_generation[0][0][0].alive = 1;
	current_generation[0][2][0].alive = 1;
	current_generation[1][1][1].alive = 1;

	/* PRINT */
    print_generation_alive_status(current_generation, size);

	int p;
	int iterations;
	iterations = 2;

	for(p = 0; p < iterations; p++)
	{
		for(i = 0; i < size; i++)
		{
			for(j = 0; j < size; j++)
			{
				for(k = 0; k < size; k++)
				{
					if (current_generation[i][j][k].alive == 1)
					{
						if(i+1 < size)
							current_generation[i+1][j][k].alive_neighbors++;
						else
							current_generation[0][j][k].alive_neighbors++;
						if(i-1 > 0)
							current_generation[i-1][j][k].alive_neighbors++;
						else
							current_generation[size-1][j][k].alive_neighbors++;
						if(j+1 < size)
							current_generation[i][j+1][k].alive_neighbors++;
						else
							current_generation[i][0][k].alive_neighbors++;
						if(j-1 > 0)
							current_generation[i][j-1][k].alive_neighbors++;
						else
							current_generation[i][size-1][k].alive_neighbors++;
						if(k+1 < size)
							current_generation[i][j][k+1].alive_neighbors++;
						else
							current_generation[i][j][0].alive_neighbors++;
						if(k-1 > 0)
							current_generation[i][j][k-1].alive_neighbors++;
						else
							current_generation[i][j][size-1].alive_neighbors++;
					}
				}
			}
		}
		for(i = 0; i < size; i++)
		{
			for(j = 0; j < size; j++)
			{
				for(k = 0; k < size; k++)
				{
					if(current_generation[i][j][k].alive == 1)
					{	
						if (current_generation[i][j][k].alive_neighbors >= 2 && current_generation[i][j][k].alive_neighbors <= 4)
						{
							next_generation[i][j][k].alive = 1;
						}
						// RESET PARAMETERS FOR NEXT ITERATION
						current_generation[i][j][k].alive = 0;
						current_generation[i][j][k].alive_neighbors = 0;
					}
					if(current_generation[i][j][k].alive == 0)
					{	
						if (current_generation[i][j][k].alive_neighbors >= 2 && current_generation[i][j][k].alive_neighbors <= 3)
						{
							next_generation[i][j][k].alive = 1;
						}
						// RESET PARAMETERS FOR NEXT ITERATION
						current_generation[i][j][k].alive_neighbors = 0;
					}
				}
			}
		}
		aux = current_generation;
		current_generation = next_generation;
		next_generation = aux;

        print_generation_alive_status(current_generation, size);
	}
}
