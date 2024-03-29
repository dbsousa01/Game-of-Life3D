/************************************************** INFO **************************************************/
/**
 * \brief		MPI implementation of a 3D version of the Game of Life by John Conway
 *				for the Parallel and Distributed Computing course at IST 16/17 2nd Semester
 *				taught by Professor José Monteiro and Professor Luís Guerra e Silva
 *
 * \author		Group 				#25
 * \author		André Mendes		#66943
 * \author		Nuno Venturinha		#67682
 * \author		Daniel Sousa		#79129
 * \version		1.0
 * \date 		19/05/2017
 */
/************************************************** INCLUDE **************************************************/
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/************************************************** DEFINE **************************************************/
#define ALIVE 								1
#define DEAD 								0
#define DOWN 								1
#define BUFFER_SIZE							200
#define HORIZONTAL 							1
#define LEFT 								2
#define NDIMS 								2
#define NEIGHBOR 							1
#define NEW 								0
#define RIGHT 								3
#define ROOT 								0
#define UP 									0
#define VERTICAL 							0
#define GLOBAL_TO_LOCAL(a, offset) 			((a) - (offset) + 1)
#define LOCAL_TO_GLOBAL(a, offset) 			((a) + (offset) - 1)
#define MOD(a, b) 							(((a) < 0) ? ((a) % (b) + (b)) : ((a) % (b)))

/************************************************** PROTOTYPES **************************************************/
struct 					border;
struct 					coordinates;
struct 					node;
void 					alloc_check 					(void *ptr);
struct node *** 		array_to_block 					(struct coordinates *array, int size_array, int size_x, int size_y);
void 					block_add_halo_horizontal 		(struct node ***block, struct border *halo, int size_halo, int position_y);
void 					block_add_halo_vertical 		(struct node ***block, struct border *halo, int size_halo, int position_x);
int 					block_alive_count 				(struct node ***block, int size_x, int size_y);
struct node *** 		block_create 					(int size_x, int size_y);
void 					block_destroy 					(struct node ***block, int size_x, int size_y);
void 					block_parameters 				(int *cart_coordinates, int *cart_dimensions, int *coordinate_x, int *coordinate_y,
															int *first_x, int *first_y, int *last_x, int *last_y, int *size_x, int *size_y, int size);
void 					block_print_cells 				(struct node ***block, int size_x, int size_y, int status);
struct coordinates * 	block_to_array 					(struct node ***block, int *size_array, int first_x, int first_y, int size_x, int size_y);
int 					border_alive_count 				(struct node **border, int size_border, int step);
struct border * 		border_to_array_horizontal 		(struct node **border, int *size_array, int size_border, int step);
struct border * 		border_to_array_vertical 		(struct node **border, int *size_array, int size_border, int step);
void 					determine_next_generation 		(struct node ***block, int size_x, int size_y);
int 					get_count_total 				(int *counts, int num_procs);
int * 					get_displs 						(int *counts, int num_procs);
void 					mark_neighbors_halo_horizontal 	(struct node ***block, int source, int destination, int size_x);
void 					mark_neighbors_halo_vertical 	(struct node ***block, int source, int destination, int size_y);
void 					node_add 						(struct node **head, short mode, short status, short x, short y, short z);
struct node * 			node_create 					(short status, short x, short y, short z);
void 					read_arguments 					(int argc, char *argv[], char **input_filename, int *iterations);
void 					read_coordinates 				(FILE *input_fd, struct node ***block, int first_x, int first_y, int last_x, int last_y);
int 					read_size 						(FILE *input_fd);

/************************************************** STRUCT BORDER **************************************************/
/** \struct
 * Structure that represents the coordinates of a block border used in the MPI communications
 * Only holds two values as the third can be inferred by knowing who is sending the message
 */
struct border
{
	short a;
	short b;
};

/************************************************** STRUCT COORDINATES **************************************************/
/** \struct
 * Structure that represents the coordinates of the cells used in the blocks.
 */
struct coordinates
{
	short x;
	short y;
	short z;
};

/************************************************** STRUCT NODE **************************************************/
/** \struct
 * Structure that represents the cells used in the blocks.
 * Holds the information relative to a cell and a pointer
 * to another one with the same [x][y] coordinates
 */
struct node
{
	short 				alive_neighbors;
	short 				status;
	struct coordinates 	coords;
	struct node 		*next;
};

/************************************************** ALLOC_CHECK **************************************************/
/**
 * Checks if a given memory allocation was successful
 *
 * @param ptr		Pointer to the memory that was allocated
 */
void alloc_check(void *ptr)
{
	if(ptr == NULL)
	{
		fprintf(stderr, "Error with memory allocation\n");
		abort();
	}
}

/************************************************** ARRAY_TO_BLOCK **************************************************/
/**
 * Turns an array of coordinates into a block of cells with the coordinates of said array
 *
 * @param array			Array of coordinates
 * @param size_array	Size of the array
 * @param size_x		Block size in x
 * @param size_y		Block size in y
 * @return 				Block
 */
struct node *** array_to_block(struct coordinates *array, int size_array, int size_x, int size_y)
{
	struct node 	***block 	= NULL;
	int 			i 			= 0;
	int 			x 			= 0;
	int 			y 			= 0;
	int 			z 			= 0;

	block = block_create(size_x, size_y);
	for(i = 0; i < size_array; i++)
	{
		x = array[i].x;
		y = array[i].y;
		z = array[i].z;
		node_add(&(block[x][y]), NEW, ALIVE, x, y, z);
	}

	return block;
}

/************************************************** BLOCK_ADD_HALO_HORIZONTAL **************************************************/
/**
 * Adds an array of coordinates corresponding to a border of a neighboring process (halo) to a given block
 *
 * @param block			Block
 * @param halo			Array of coordinates
 * @param size_halo		Size of the array
 * @param position_y	Coordinate in which to add the array
 */
void block_add_halo_horizontal(struct node ***block, struct border *halo, int size_halo, int position_y)
{
	int 	i 	= 0;
	int 	x 	= 0;
	int 	y 	= 0;
	int 	z 	= 0;

	y = position_y;
	for(i = 0; i < size_halo; i++)
	{
		x = halo[i].a;
		z = halo[i].b;
		node_add(&(block[x][y]), NEW, ALIVE, x, y, z);
	}
}

/************************************************** BLOCK_ADD_HALO_VERTICAL **************************************************/
/**
 * Adds an array of coordinates corresponding to a border of a neighboring process (halo) to a given block
 *
 * @param block			Block
 * @param halo			Array of coordinates
 * @param size_halo		Size of the array
 * @param position_x	Coordinate in which to add the array
 */
void block_add_halo_vertical(struct node ***block, struct border *halo, int size_halo, int position_x)
{
	int 	i 	= 0;
	int 	x 	= 0;
	int 	y 	= 0;
	int 	z 	= 0;

	x = position_x;
	for(i = 0; i < size_halo; i++)
	{
		y = halo[i].a;
		z = halo[i].b;
		node_add(&(block[x][y]), NEW, ALIVE, x, y, z);
	}
}

/************************************************** BLOCK_ALIVE_COUNT **************************************************/
/**
 * Counts and return the number of alive cells in a given block
 *
 * @param block			Block
 * @param size_x		Block size in x
 * @param size_y		Block size in y
 * @return 				Count
 */
int block_alive_count(struct node ***block, int size_x, int size_y)
{
	struct node 	*aux 	= NULL;
	int 			count 	= 0;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = 1; x < (size_x - 1); x++)
	{
		for(y = 1; y < (size_y - 1); y++)
		{
			aux = block[x][y];
			while(aux != NULL)
			{
				if(aux->status == ALIVE)
				{
					count++;
				}
				aux = aux->next;
			}
		}
	}

	return count;
}

/************************************************** BLOCK_CREATE **************************************************/
/**
 * Creates a block with the given sizes in x and y and returns a pointer to it
 *
 * @param size_x		Block size in x
 * @param size_y		Block size in y
 * @return 				Block
 */
struct node *** block_create(int size_x, int size_y)
{
	struct node 	***block 		= NULL;
	struct node 	**block_mem 	= NULL;
	int 			x 				= 0;

	block = (struct node ***) calloc(size_x, sizeof(struct node **));
	block_mem = (struct node **) calloc(size_x * size_y, sizeof(struct node *));
	for(x = 0; x < size_x; x++)
	{
		block[x] = &block_mem[x * size_y];
	}

	return block;
}

/************************************************** BLOCK_DESTROY **************************************************/
/**
 * Destroys a given block with the given sizes in x and y
 *
 * @param block			Block
 * @param size_x		Block size in x
 * @param size_y		Block size in y
 */
void block_destroy(struct node ***block, int size_x, int size_y)
{
	struct node 	*aux 	= NULL;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = 0; x < size_x; x++)
	{
		for(y = 0; y < size_y; y++)
		{
			while(block[x][y] != NULL)
			{
				aux = block[x][y];
				block[x][y] = block[x][y]->next;
				free(aux);
			}
		}
	}
	free(block[0]);
	free(block);
}

/************************************************** BLOCK_PARAMETERS **************************************************/
/**
 * Computes the sizes and delimiters for the block to be handled by a given processor
 *
 * @param cart_coordinates		Coordinates of the process in the process grid
 * @param cart_dimensions		Dimensions of the process grid
 * @param coordinate_x			x-coordinate of the process
 * @param coordinate_y			x-coordinate of the process
 * @param first_x				First x-coordinate of the data to be handled by the process
 * @param first_y				First y-coordinate of the data to be handled by the process
 * @param last_x				Last x-coordinate of the data to be handled by the process
 * @param last_y				Last y-coordinate of the data to be handled by the process
 * @param size_x				Size of the x-coordinate block of the data to be handled by the process
 * @param size_y				Size of the y-coordinate block of the data to be handled by the process
 * @param size					Size of the cube
 */
void block_parameters(int *cart_coordinates, int *cart_dimensions, int *coordinate_x, int *coordinate_y,
						int *first_x, int *first_y, int *last_x, int *last_y, int *size_x, int *size_y, int size)
{
	int 	num_cols 		= 0;
	int 	num_rows 		= 0;
	int 	remainder_x 	= 0;
	int 	remainder_y 	= 0;

	/* x and y coordinates of each process */
	(*coordinate_x) = cart_coordinates[1];
	(*coordinate_y) = cart_coordinates[0];
	/* Number of columns and rows of the cartesian topology */
	num_cols = cart_dimensions[1];
	num_rows = cart_dimensions[0];
	/* x and y sizes of the blocks */
	(*size_x) = size / num_cols;
	(*size_y) = size / num_rows;
	/* Remaining columns and rows if the problem size isn't evenly divisible */
	remainder_x = size % num_cols;
	remainder_y = size % num_rows;
	/* x and y positions where each block starts */
	(*first_x) = ((*size_x) * (*coordinate_x)) + remainder_x;
	(*first_y) = ((*size_y) * (*coordinate_y)) + remainder_y;
	/* x and y positions where each block ends */
	(*last_x) = (*first_x) + (*size_x) - 1;
	(*last_y) = (*first_y) + (*size_y) - 1;
	/* If the process is on the first column add the remainder of the columns to it */
	if((*coordinate_x) == 0)
	{
		(*first_x) -= remainder_x;
		(*size_x) += remainder_x;
	}
	/* If the process is on the first row add the remainder of the rows to it */
	if((*coordinate_y) == 0)
	{
		(*first_y) -= remainder_y;
		(*size_y) += remainder_y;
	}
	/* Increase the size on both the x and y axis by 2 to store the halos of the neighboring processes */
	(*size_x) += 2;
	(*size_y) += 2;
}

/************************************************** BLOCK_PRINT_CELLS **************************************************/
/**
 * Prints the cells of a given block with the given status
 *
 * @param block			Block
 * @param size_x		Block size in x
 * @param size_y		Block size in y
 * @param status		Status of the cells to print
 */
void block_print_cells(struct node ***block, int size_x, int size_y, int status)
{
	struct node 	*aux 	= NULL;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = 0; x < size_x; x++)
	{
		for(y = 0; y < size_y; y++)
		{
			aux = block[x][y];
			while(aux != NULL)
			{
				if(aux->status == status)
				{
					fprintf(stdout, "%d %d %d\n", aux->coords.x, aux->coords.y, aux->coords.z);
				}
				aux = aux->next;
			}
		}
	}
}

/************************************************** BLOCK_TO_ARRAY **************************************************/
/**
 * Turns a block of cells into an array of coordinates of the cells of said block
 *
 * @param block			Block
 * @param size_array	Size of the array
 * @param first_x		First x-coordinate of the data handled by the process (Used to turn the local indexes into global ones)
 * @param first_y		First y-coordinate of the data handled by the process (Used to turn the local indexes into global ones)
 * @param size_x		Block size in x
 * @param size_y		Block size in y
 * @return 				Array
 */
struct coordinates * block_to_array(struct node ***block, int *size_array, int first_x, int first_y, int size_x, int size_y)
{
	struct coordinates 	*array 			= NULL;
	struct coordinates 	*ptr 			= NULL;
	struct node 		*aux 			= NULL;
	int 				x 				= 0;
	int 				y 				= 0;

	(*size_array) = block_alive_count(block, size_x, size_y);
	array = (struct coordinates *) calloc((*size_array), sizeof(struct coordinates));

	ptr = array;
	for(x = 1; x < (size_x - 1); x++)
	{
		for(y = 1; y < (size_y - 1); y++)
		{
			aux = block[x][y];
			while(aux != NULL)
			{
				if(aux->status == ALIVE)
				{
					(*ptr).x = LOCAL_TO_GLOBAL((aux->coords.x), first_x);
					(*ptr).y = LOCAL_TO_GLOBAL((aux->coords.y), first_y);
					(*ptr).z = aux->coords.z;
					ptr++;
				}
				aux = aux->next;
			}
		}
	}

	return array;
}

/************************************************** BORDER_ALIVE_COUNT **************************************************/
/**
 * Counts and return the number of alive cells in a given border
 *
 * @param border		Border
 * @param size_border	Size of the border
 * @param step			Step that separates elements of said border (Allocated in contiguous memory, different in x and y)
 * @return 				Count
 */
int border_alive_count(struct node **border, int size_border, int step)
{
	struct node 	**ptr 	= NULL;
	struct node 	*aux 	= NULL;
	int 			count 	= 0;
	int 			i 		= 0;

	ptr = border;
	for(i = 0; i < size_border; i++)
	{
		aux = (*ptr);
		while(aux != NULL)
		{
			if(aux->status == ALIVE)
			{
				count++;
			}
			aux = aux->next;
		}
		ptr += step;
	}

	return count;
}

/************************************************** BORDER_TO_ARRAY_HORIZONTAL **************************************************/
/**
 * Turns a border of cells into an array of coordinates of the cells of said border
 *
 * @param border		Border
 * @param size_array	Size of the array
 * @param size_border	Size of the border
 * @param step			Step that separates elements of said border (Allocated in contiguous memory, different in x and y)
 * @return 				Array
 */
struct border * border_to_array_horizontal(struct node **border, int *size_array, int size_border, int step)
{
	struct border 	*array 			= NULL;
	struct border 	*ptr_array 		= NULL;
	struct node 	**ptr_border 	= NULL;
	struct node 	*aux 			= NULL;
	int 			i 				= 0;

	(*size_array) = border_alive_count(border, size_border, step);
	array = (struct border *) calloc((*size_array), sizeof(struct border));
	ptr_array = array;
	ptr_border = border;
	for(i = 0; i < size_border; i++)
	{
		aux = (*ptr_border);
		while(aux != NULL)
		{
			if(aux->status == ALIVE)
			{
				(*ptr_array).a = aux->coords.x;
				(*ptr_array).b = aux->coords.z;
				ptr_array++;
			}
			aux = aux->next;
		}
		(ptr_border) += step;
	}

	return array;
}

/************************************************** BORDER_TO_ARRAY_VERTICAL **************************************************/
/**
 * Turns a border of cells into an array of coordinates of the cells of said border
 *
 * @param border		Border
 * @param size_array	Size of the array
 * @param size_border	Size of the border
 * @param step			Step that separates elements of said border (Allocated in contiguous memory, different in x and y)
 * @return 				Array
 */
struct border * border_to_array_vertical(struct node **border, int *size_array, int size_border, int step)
{
	struct border 	*array 			= NULL;
	struct border 	*ptr_array 		= NULL;
	struct node 	**ptr_border 	= NULL;
	struct node 	*aux 			= NULL;
	int 			i 				= 0;

	(*size_array) = border_alive_count(border, size_border, step);
	array = (struct border *) calloc((*size_array), sizeof(struct border));
	ptr_array = array;
	ptr_border = border;
	for(i = 0; i < size_border; i++)
	{
		aux = (*ptr_border);
		while(aux != NULL)
		{
			if(aux->status == ALIVE)
			{
				(*ptr_array).a = aux->coords.y;
				(*ptr_array).b = aux->coords.z;
				ptr_array++;
			}
			aux = aux->next;
		}
		(ptr_border) += step;
	}

	return array;
}

/************************************************** DETERMINE_NEXT_GENERATION **************************************************/
/**
 * Iterates through all the cells in the given block and determines whether they live or die in the next generation while
 * removing the dead cells.
 *
 * @param block 	Block
 * @param size_x	Block size in x
 * @param size_y	Block size in y
 */
void determine_next_generation(struct node ***block, int size_x, int size_y)
{
	struct node 	**ptr 	= NULL;
	struct node 	*aux 	= NULL;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = 0; x < size_x; x++)
	{
		for(y = 0; y < size_y; y++)
		{
			ptr = &block[x][y];
			aux = (*ptr);
			while(aux != NULL)
			{
				if(aux->status == ALIVE)
				{
					if(aux->alive_neighbors < 2 || aux->alive_neighbors > 4)
					{
						(*ptr) = aux->next;
						free(aux);
					}
					else
					{
						aux->alive_neighbors = 0;
						ptr = &aux->next;
					}
				}
				else
				{
					if(aux->alive_neighbors == 2 || aux->alive_neighbors == 3)
					{
						aux->status = ALIVE;
						aux->alive_neighbors = 0;
						ptr = &aux->next;
					}
					else
					{
						(*ptr) = aux->next;
						free(aux);
					}
				}
				aux = (*ptr);
			}
		}
	}
}

/************************************************** GET_COUNT_TOTAL **************************************************/
/**
 * Returns the sum of the cell count each process sends to the root
 *
 * @param counts 		Array of cell counts
 * @param num_procs 	Number of processes
 * @return 				Count
 */
int get_count_total(int *counts, int num_procs)
{
	int 	count 	= 0;
	int 	i 		= 0;

 	for(i = 0; i < num_procs; i++)
	{
		count += counts[i];
	}

	return count;
}

/************************************************** GET_COUNT_TOTAL **************************************************/
/**
 * Returns the displacements of the array of cells received by the root and sent by all processes
 *
 * @param counts 		Array of cell counts
 * @param num_procs 	Number of processes
 * @return 				Array of displacements
 */
int * get_displs(int *counts, int num_procs)
{
	int 	*displs 	= NULL;
	int 	i 			= 0;

	displs = (int *) calloc(num_procs, sizeof(int));
	for(i = 1; i < num_procs; i++)
	{
		displs[i] = displs[i-1] + counts[i-1];
	}

	return displs;
}

/************************************************** MARK_NEIGHBORS_HALO_HORIZONTAL **************************************************/
/**
 * Marks the neighbors of a given halo of a given block
 *
 * @param block 		Block
 * @param source 		y-coordinate of halo
 * @param destination 	y-coordinate of neighbors of halo
 * @param size_x		Block size in x
 */
void mark_neighbors_halo_horizontal(struct node ***block, int source, int destination, int size_x)
{
	struct node 	*aux 	= NULL;
	int 			x 		= NULL;

	for(x = 1; x < (size_x - 1); x++)
	{
		aux = block[x][source];
		while(aux != NULL)
		{
			if(aux->status == ALIVE)
			{
				node_add(&(block[x][destination]), NEIGHBOR, DEAD, x, destination, aux->coords.z);
			}
			aux = aux->next;
		}
	}
}

/************************************************** MARK_NEIGHBORS_HALO_VERTICAL **************************************************/
/**
 * Marks the neighbors of a given halo of a given block
 *
 * @param block 		Block
 * @param source 		x-coordinate of halo
 * @param destination 	x-coordinate of neighbors of halo
 * @param size_y		Block size in y
 */
void mark_neighbors_halo_vertical(struct node ***block, int source, int destination, int size_y)
{
	struct node 	*aux 	= NULL;
	int 			y 		= NULL;

	for(y = 1; y < (size_y - 1); y++)
	{
		aux = block[source][y];
		while(aux != NULL)
		{
			if(aux->status == ALIVE)
			{
				node_add(&(block[destination][y]), NEIGHBOR, DEAD, destination, y, aux->coords.z);
			}
			aux = aux->next;
		}
	}
}

/************************************************** NODE_ADD **************************************************/
/**
 * Adds a node to the given list
 *
 * @param head 		Head of the list where to insert the node
 * @param mode 		A way to differentiate whether we're adding neighbors of alive cells or alive cells themselves.
 *					It could be suppressed by creating an almost identical function for each case
 * @param status 	Status of the cell to add
 * @param z 		z-coordinate of the cell to add
 */
void node_add(struct node **head, short mode, short status, short x, short y, short z)
{
	struct node 	*aux 	= NULL;
	struct node 	*new 	= NULL;

	if(((*head) == NULL) || ((*head)->coords.z > z))
	{
		new = node_create(status, x, y, z);
		new->alive_neighbors += mode;
		new->next = (*head);
		(*head) = new;
	}
	else if((*head)->coords.z == z)
	{
		if(status == ALIVE)
		{
			(*head)->status = ALIVE;
		}
		else
		{
			(*head)->alive_neighbors += mode;
		}
	}
	else
	{
		aux = (*head);
		while((aux->next != NULL) && (aux->next->coords.z <= z))
		{
			aux = aux->next;
		}
		if(aux->coords.z == z)
		{
			if(status == ALIVE)
			{
				aux->status = ALIVE;
			}
			else
			{
				aux->alive_neighbors += mode;
			}
		}
		else
		{
			new = node_create(status, x, y, z);
			new->alive_neighbors += mode;
			new->next = aux->next;
			aux->next = new;
		}
	}
}

/************************************************** NODE_CREATE **************************************************/
/**
 * Creates a node that represent a cell in the game
 *
 * @param status 	Status of the cell to create
 * @param z 		z-coordinate of the cell to create
 * @return 			Node
 */
struct node * node_create(short status, short x, short y, short z)
{
	struct node 	*new 	= NULL;

	new = (struct node *) calloc(1, sizeof(struct node));
	alloc_check(new);

	new->alive_neighbors = 0;
	new->status = status;
	new->coords.x = x;
	new->coords.y = y;
	new->coords.z = z;
	new->next = NULL;

	return new;
}

/************************************************** READ_ARGUMENTS **************************************************/
/**
 * Checks if the command line arguments verify the specifications
 *
 * @param argc 				Command line argument count
 * @param argv				Command line arguments
 * @param input_filename	Name of the file specified in the arguments
 * @param iterations 		Number of iterations specified in the arguments
 */
void read_arguments(int argc, char *argv[], char **input_filename, int *iterations)
{
	FILE 	*input_fd 	= NULL;

	if(argc != 3)
	{
		fprintf(stderr, "Program is run with %s [name-of-input-file] [number-of-iterations]\n", argv[0]);
		exit(-1);
	}

	(*input_filename) = argv[1];
	input_fd = fopen((*input_filename), "r");
	if(input_fd == NULL)
	{
		fprintf(stderr, "Error opening given file\n");
		exit(-1);
	}
	fclose(input_fd);

	(*iterations) = atoi(argv[2]);
	if((*iterations) <= 0)
	{
		fprintf(stderr, "The number of iterations must be >= 1\n");
		exit(-1);
	}
}

/************************************************** READ_COORDINATES **************************************************/
/**
 * Reads the input file and stores the cells assigned to this process in the given block
 *
 * @param input_fd 			File
 * @param block 			Block
 * @param first_x			First x-coordinate of the data handled by the process
 * @param first_y			First y-coordinate of the data handled by the process
 * @param last_x			Last x-coordinate of the data handled by the process
 * @param last_y			Last y-coordinate of the data handled by the process
 */
void read_coordinates(FILE *input_fd, struct node ***block, int first_x, int first_y, int last_x, int last_y)
{
	char 	buffer[BUFFER_SIZE] 	= {0};
	int 	x 						= 0;
	int 	y 						= 0;
	int 	z 						= 0;

	while(fgets(buffer, BUFFER_SIZE, input_fd) != NULL)
	{
		if((sscanf(buffer,"%d %d %d", &x, &y, &z)) != 3)
		{
			fprintf(stderr, "Input file does not match specifications\n");
			exit(-1);
		}
		/* Only add the cells that correspond to the coordinates assigned to this block */
		if((x >= first_x) && (x <= last_x) && (y >= first_y) && (y <= last_y))
		{
			x = GLOBAL_TO_LOCAL(x, first_x);
			y = GLOBAL_TO_LOCAL(y, first_y);
			node_add(&(block[x][y]), NEW, ALIVE, x, y, z);
		}
	}
}

/************************************************** READ_SIZE **************************************************/
/**
 * Reads the input file and returns the declared size of the sides of the cube
 *
 * @param input_fd 			File
 * @return size 			Size of the cube
 */
int read_size(FILE *input_fd)
{
	char 	buffer[BUFFER_SIZE] 	= {0};
	int 	size 					= 0;

	fgets(buffer, BUFFER_SIZE, input_fd);
	if((sscanf(buffer, "%d", &size)) != 1)
	{
		fprintf(stderr, "Input file does not match specifications\n");
		exit(-1);
	}

	return size;
}

/************************************************** MAIN **************************************************/
int main(int argc, char *argv[])
{
	FILE 				*input_fd 					= NULL;

	MPI_Comm 			MPI_COMM_CUBE;
	MPI_Datatype 		MPI_BORDER;
	MPI_Datatype 		MPI_COORDINATES;
	MPI_Request 		request_recv[4];
	MPI_Request 		request_send[4];
	MPI_Status 			status;

	struct border 		*buffer_border_up 			= NULL;
	struct border 		*buffer_border_down 		= NULL;
	struct border 		*buffer_border_left 		= NULL;
	struct border 		*buffer_border_right 		= NULL;
	struct border 		*buffer_halo_up 			= NULL;
	struct border 		*buffer_halo_down 			= NULL;
	struct border 		*buffer_halo_left 			= NULL;
	struct border 		*buffer_halo_right 			= NULL;

	struct coordinates 	*buffer_block 				= NULL;
	struct coordinates 	*buffer_gather 				= NULL;

	struct node 		***block 					= NULL;
	struct node 		*aux 						= NULL;

	char 				*input_filename 			= NULL;

	int 				*displs 					= NULL;
	int 				*recvcounts 				= NULL;
	int 				cart_coordinates[NDIMS] 	= {0, 0};
	int 				cart_dimensions[NDIMS] 		= {0, 0};
	int 				cart_periodicity[NDIMS] 	= {1, 1};
	int 				coordinate_x 				= 0;
	int 				coordinate_y 				= 0;
	int 				count_block 				= 0;
	int 				count_border_up 			= 0;
	int 				count_border_down 			= 0;
	int 				count_border_left 			= 0;
	int 				count_border_right 			= 0;
	int 				count_halo_up 				= 0;
	int 				count_halo_down 			= 0;
	int 				count_halo_left 			= 0;
	int 				count_halo_right 			= 0;
	int 				count_total 				= 0;
	int 				iterations 					= 0;
	int 				neighbor_down 				= 0;
	int 				neighbor_up 				= 0;
	int 				neighbor_left 				= 0;
	int 				neighbor_right 				= 0;
	int 				num_procs 					= 0;
	int 				rank 						= 0;
	int 				size 						= 0;
	int 				size_x 						= 0;
	int 				size_y 						= 0;
	int 				first_x 					= 0;
	int 				first_y 					= 0;
	int 				last_x 						= 0;
	int 				last_y 						= 0;
	int 				x 							= 0;
	int 				y 							= 0;
	int 				z 							= 0;

	/* Initialize MPI */
	MPI_Init(&argc, &argv);
	MPI_Barrier(MPI_COMM_WORLD);

	/* Get the number of processes created by MPI and their rank */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* Create a 2D cartesian topology of the processes */
	MPI_Dims_create(num_procs, NDIMS, cart_dimensions);
	MPI_Cart_create(MPI_COMM_WORLD, NDIMS, cart_dimensions, cart_periodicity, 1, &MPI_COMM_CUBE);

	/* Get relevant data from the created topology */
	MPI_Cart_coords(MPI_COMM_CUBE, rank, NDIMS, cart_coordinates);
	MPI_Cart_rank(MPI_COMM_CUBE, cart_coordinates, &rank);
	MPI_Cart_shift(MPI_COMM_CUBE, VERTICAL, 1, &neighbor_up, &neighbor_down);
	MPI_Cart_shift(MPI_COMM_CUBE, HORIZONTAL, 1, &neighbor_left, &neighbor_right);

	/* Define and commit the type used for border and halo swapping */
	MPI_Type_contiguous(2, MPI_SHORT, &MPI_BORDER);
	MPI_Type_commit(&MPI_BORDER);
	MPI_Type_contiguous(3, MPI_SHORT, &MPI_COORDINATES);
	MPI_Type_commit(&MPI_COORDINATES);

	/* Check command line arguments */
	read_arguments(argc, argv, &input_filename, &iterations);
	/* Open input file */
	input_fd = fopen(input_filename, "r");
	/* Read problem size */
	size = read_size(input_fd);

	/* Compute parameters for the block decomposition */
	block_parameters(cart_coordinates, cart_dimensions, &coordinate_x, &coordinate_y, &first_x, &first_y, &last_x, &last_y, &size_x, &size_y, size);

	/* Create the local block */
	block = block_create(size_x, size_y);
	/* Read the input and add to the block only the cells with coordinates assigned to it */
	read_coordinates(input_fd, block, first_x, first_y, last_x, last_y);

	/* Problem solving loop */
	while(iterations > 0)
	{
		/* Turn the borders into arrays */
		buffer_border_up = border_to_array_horizontal(&(block[0][1]), &count_border_up, size_x, size_y);
		buffer_border_down = border_to_array_horizontal(&(block[0][size_y - 2]), &count_border_down, size_x, size_y);
		buffer_border_left = border_to_array_vertical(&(block[1][0]), &count_border_left, size_y, 1);
		buffer_border_right = border_to_array_vertical(&(block[size_x - 2][0]), &count_border_right, size_y, 1);

		/* Start the asynchronous send */
		MPI_Isend(buffer_border_up, count_border_up, MPI_BORDER, neighbor_up, UP, MPI_COMM_CUBE, &request_send[0]);
		MPI_Isend(buffer_border_down, count_border_down, MPI_BORDER, neighbor_down, DOWN, MPI_COMM_CUBE, &request_send[1]);
		MPI_Isend(buffer_border_left, count_border_left, MPI_BORDER, neighbor_left, LEFT, MPI_COMM_CUBE, &request_send[2]);
		MPI_Isend(buffer_border_right, count_border_right, MPI_BORDER, neighbor_right, RIGHT, MPI_COMM_CUBE, &request_send[3]);

		/* Process a third of the neighbors to guarantee that the sends take place before continuing and not waste time */
		for(x = 1; x < (size_x/3); x++)
		{
			for(y = 1; y < (size_y - 1); y++)
			{
				aux = block[x][y];
				while(aux != NULL)
				{
					if(aux->status == ALIVE)
					{
						z = aux->coords.z;
						node_add(&(block[x+1][y]), NEIGHBOR, DEAD, x+1, y, z);
						node_add(&(block[x-1][y]), NEIGHBOR, DEAD, x-1, y, z);
						node_add(&(block[x][y+1]), NEIGHBOR, DEAD, x, y+1, z);
						node_add(&(block[x][y-1]), NEIGHBOR, DEAD, x, y-1, z);
						node_add(&(block[x][y]), NEIGHBOR, DEAD, x, y, MOD((z+1), size));
						node_add(&(block[x][y]), NEIGHBOR, DEAD, x, y, MOD((z-1), size));
					}
					aux = aux->next;
				}
			}
		}

		/* Probe the received halos form the neighbors, allocate memory and start the asynchronous receive */
		MPI_Probe(neighbor_up, DOWN, MPI_COMM_CUBE, &status);
		MPI_Get_count(&status, MPI_BORDER, &count_halo_up);
		buffer_halo_up = (struct border *) calloc(count_halo_up, sizeof(struct border));
		MPI_Irecv(buffer_halo_up, count_halo_up, MPI_BORDER, neighbor_up, DOWN, MPI_COMM_CUBE, &request_recv[0]);

		MPI_Probe(neighbor_down, UP, MPI_COMM_CUBE, &status);
		MPI_Get_count(&status, MPI_BORDER, &count_halo_down);
		buffer_halo_down = (struct border *) calloc(count_halo_down, sizeof(struct border));
		MPI_Irecv(buffer_halo_down, count_halo_down, MPI_BORDER, neighbor_down, UP, MPI_COMM_CUBE, &request_recv[1]);

		MPI_Probe(neighbor_left, RIGHT, MPI_COMM_CUBE, &status);
		MPI_Get_count(&status, MPI_BORDER, &count_halo_left);
		buffer_halo_left = (struct border *) calloc(count_halo_left, sizeof(struct border));
		MPI_Irecv(buffer_halo_left, count_halo_left, MPI_BORDER, neighbor_left, RIGHT, MPI_COMM_CUBE, &request_recv[2]);

		MPI_Probe(neighbor_right, LEFT, MPI_COMM_CUBE, &status);
		MPI_Get_count(&status, MPI_BORDER, &count_halo_right);
		buffer_halo_right = (struct border *) calloc(count_halo_right, sizeof(struct border));
		MPI_Irecv(buffer_halo_right, count_halo_right, MPI_BORDER, neighbor_right, LEFT, MPI_COMM_CUBE, &request_recv[3]);

		/* Process a third of the neighbors to guarantee that the receives take place before continuing and not waste time */
		for(x = (size_x/3); x < (2*(size_x/3)); x++)
		{
			for(y = 1; y < (size_y - 1); y++)
			{
				aux = block[x][y];
				while(aux != NULL)
				{
					if(aux->status == ALIVE)
					{
						z = aux->coords.z;
						node_add(&(block[x+1][y]), NEIGHBOR, DEAD, x+1, y, z);
						node_add(&(block[x-1][y]), NEIGHBOR, DEAD, x-1, y, z);
						node_add(&(block[x][y+1]), NEIGHBOR, DEAD, x, y+1, z);
						node_add(&(block[x][y-1]), NEIGHBOR, DEAD, x, y-1, z);
						node_add(&(block[x][y]), NEIGHBOR, DEAD, x, y, MOD((z+1), size));
						node_add(&(block[x][y]), NEIGHBOR, DEAD, x, y, MOD((z-1), size));
					}
					aux = aux->next;
				}
			}
		}

		/* Wait for the receives to finish as they are needed to continue */
		MPI_Waitall(4, request_recv, MPI_STATUSES_IGNORE);
		block_add_halo_horizontal(block, buffer_halo_up, count_halo_up, 0);
		free(buffer_halo_up);
		block_add_halo_horizontal(block, buffer_halo_down, count_halo_down, size_y - 1);
		free(buffer_halo_down);
		block_add_halo_vertical(block, buffer_halo_left, count_halo_left, 0);
		free(buffer_halo_left);
		block_add_halo_vertical(block, buffer_halo_right, count_halo_right, size_x - 1);
		free(buffer_halo_right);

		/* Finish processing the neighbors */
		for(x = (2*(size_x/3)); x < (size_x - 1); x++)
		{
			for(y = 1; y < (size_y - 1); y++)
			{
				aux = block[x][y];
				while(aux != NULL)
				{
					if(aux->status == ALIVE)
					{
						z = aux->coords.z;
						node_add(&(block[x+1][y]), NEIGHBOR, DEAD, x+1, y, z);
						node_add(&(block[x-1][y]), NEIGHBOR, DEAD, x-1, y, z);
						node_add(&(block[x][y+1]), NEIGHBOR, DEAD, x, y+1, z);
						node_add(&(block[x][y-1]), NEIGHBOR, DEAD, x, y-1, z);
						node_add(&(block[x][y]), NEIGHBOR, DEAD, x, y, MOD((z+1), size));
						node_add(&(block[x][y]), NEIGHBOR, DEAD, x, y, MOD((z-1), size));
					}
					aux = aux->next;
				}
			}
		}
		mark_neighbors_halo_vertical(block, 0, 1, size_y);
		mark_neighbors_halo_vertical(block, size_x - 1, size_x - 2, size_y);
		mark_neighbors_halo_horizontal(block, 0, 1, size_x);
		mark_neighbors_halo_horizontal(block, size_y - 1, size_y - 2, size_x);
		determine_next_generation(block, size_x, size_y);
		iterations--;

		/* Synchronization point for the sends just to guarantee no overlapping occurs between iterations */
		MPI_Waitall(4, request_send, MPI_STATUSES_IGNORE);
		free(buffer_border_up);
		free(buffer_border_down);
		free(buffer_border_left);
		free(buffer_border_right);
	}

	/* Turns the assigned block of the process into an array to send to the root */
	buffer_block = block_to_array(block, &count_block, first_x, first_y, size_x, size_y);
	block_destroy(block, size_x, size_y);

	/* Root process gathers the sizes of the arrays sent by all the processes */
	if(rank == ROOT)
	{
		recvcounts = (int *) calloc(num_procs, sizeof(int));
	}
	MPI_Gather(&count_block, 1, MPI_INT, recvcounts, 1, MPI_INT, ROOT, MPI_COMM_CUBE);

	/* Root process computes the necessary values and gathers the arrays of all the processes */
	if(rank == ROOT)
	{
		count_total = get_count_total(recvcounts, num_procs);
		displs = get_displs(recvcounts, num_procs);
		buffer_gather = (struct coordinates *) calloc(count_total, sizeof(struct coordinates));
	}

	MPI_Gatherv(buffer_block, count_block, MPI_COORDINATES, buffer_gather, recvcounts, displs, MPI_COORDINATES, ROOT, MPI_COMM_CUBE);
	free(buffer_block);

	/* Root process turns the received arrays into the final block with the solution of the problem and prints it */
	if(rank == ROOT)
	{
		free(displs);
		free(recvcounts);
		block = array_to_block(buffer_gather, count_total, size, size);
		free(buffer_gather);
		block_print_cells(block, size, size, ALIVE);
		block_destroy(block, size, size);
	}

	/* Finalize MPI */
	MPI_Barrier(MPI_COMM_CUBE);
	MPI_Finalize();

	return 0;
}
