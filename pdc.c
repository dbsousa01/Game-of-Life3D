
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


int main(int argc, char *argv[])
{
	struct cell
	{
		bool alive;
		int alive_neighbors;
		int x;
		int y;
		int z;
	};
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
				current_generation[i][j][k].x = k;
				current_generation[i][j][k].y = j;
				current_generation[i][j][k].z = i;
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
				next_generation[i][j][k].x = k;
				next_generation[i][j][k].y = j;
				next_generation[i][j][k].z = i;
			}
		}
	}

	current_generation[0][0][0].alive = 1;
	current_generation[0][2][0].alive = 1;
	current_generation[1][1][1].alive = 1;

	/* PRINT */
	for(i = 0; i < size; i++)
	{
		printf("\nGeneration 0 Slice %d\n", i+1);
		for(j = 0; j < size; j++)
		{
			for(k = 0; k < size; k++)
			{
				/* COORDINATES */
				// printf("(%d,%d,%d) ", current_generation[i][j][k].x, current_generation[i][j][k].y, current_generation[i][j][k].z);
				/* ALIVE STATUS */
				printf("(%d) ", current_generation[i][j][k].alive);
			}
			printf("\n");
		}
	}
	
	// for(i = 0; i < size; i++)
	// {
	// 	printf("\nCurrent Generation Slice %d\n", i+1);
	// 	for(j = 0; j < size; j++)
	// 	{
	// 		for(k = 0; k < size; k++)
	// 		{
	// 			/* COORDINATES */
	// 			// printf("(%d,%d,%d) ", current_generation[i][j][k].x, current_generation[i][j][k].y, current_generation[i][j][k].z);
	// 			/* ALIVE STATUS */
	// 			printf("(%d,%d) ", current_generation[i][j][k].alive,current_generation[i][j][k].alive_neighbors);
	// 		}
	// 		printf("\n");
	// 	}
	// }
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
		for(i = 0; i < size; i++)
		{
			printf("\nGeneration %d Slice %d\n", p+1,i+1);
			for(j = 0; j < size; j++)
			{
				for(k = 0; k < size; k++)
				{
					/* COORDINATES */
					// printf("(%d,%d,%d) ", current_generation[i][j][k].x, current_generation[i][j][k].y, current_generation[i][j][k].z);
					/* ALIVE STATUS */
					printf("(%d) ", current_generation[i][j][k].alive);
				}
				printf("\n");
			}
		}
	}

	// /* PRINT */
	// for(i = 0; i < size; i++)
	// {
	// 	printf("\n1 - Current Generation Slice %d\n", i+1);
	// 	for(j = 0; j < size; j++)
	// 	{
	// 		for(k = 0; k < size; k++)
	// 		{
	// 			/* COORDINATES */
	// 			// printf("(%d,%d,%d) ", current_generation[i][j][k].x, current_generation[i][j][k].y, current_generation[i][j][k].z);
	// 			/* ALIVE STATUS */
	// 			printf("(%d) ", current_generation[i][j][k].alive);
	// 		}
	// 		printf("\n");
	// 	}
	// }


}
