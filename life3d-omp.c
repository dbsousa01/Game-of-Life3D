/************************************************** INFO **************************************************/
/**
 * \brief		OpenMP implementation of a 3D version of the Game of Life by John Conway
 *				for the Parallel and Distributed Computing course at IST 16/17 2nd Semester
 *				taught by Professor José Monteiro and Professor Luís Guerra e Silva
 *
 * \author		Group 				#25
 * \author		André Mendes		#66943
 * \author		Nuno Venturinha		#67682
 * \author		Daniel Sousa		#79129
 * \version		1.0
 * \date 		07/04/2017
 */
/************************************************** INCLUDE **************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/************************************************** DEFINE **************************************************/
/************************************************** CONSTANTS **************************************************/
#define ALIVE 				1		/** \def 	Macro to differentiate alive from dead cells 	*/
#define BUFFER_SIZE			200		/** \def 	Size of the file reading buffer 				*/
#define DEAD 				0		/** \def 	Macro to differentiate alive from dead cells 	*/
#define NEIGHBOR 			1		/** \def 	Macro to differentiate neighbor from new cells 	*/
#define NEW 				0		/** \def 	Macro to differentiate neighbor from new cells 	*/

/************************************************** OPERATORS **************************************************/
#define MOD(a, b) 			(((a) < 0) ? ((a) % (b) + (b)) : ((a) % (b)))

/************************************************** PROTOTYPES **************************************************/
struct 				node;
void 				alloc_check 				(void *ptr);
struct node *** 	cube_create 				(int size);
void 				cube_destroy 				(struct node ***cube, int size);
void 				cube_print 					(struct node ***cube, int size);
void 				cube_purge 					(struct node ***cube, int size);
void 				determine_next_generation 	(struct node ***cube, int size);
void 				mark_neighbors 				(struct node ***cube, int size);
void 				node_add 					(struct node **head, short mode, short status, int z);
struct node * 		node_create 				(short status, int z);
void 				read_arguments 				(int argc, char *argv[], char **input_filename, int *iterations);
void 				read_coordinates 			(FILE *input_fd, struct node ***cube);
int 				read_size 					(FILE *input_fd);

/************************************************** STRUCT NODE **************************************************/
/** \struct
 * Structure that represents the cells used in the cube.
 * Holds the information related to a given cell and a pointer
 * to another one with the same [x][y] coordinates
 */
struct node
{
	short 			alive_neighbors; 	/**<	Number of alive neighbors this cell has 					*/
	short 			status; 			/**<	Whether this cell is alive or dead 							*/
	short 			z; 					/**<	z-coordinate of this cell 									*/
	struct node 	*next; 				/**< 	Pointer to another cell with the same [x][y] coordinates 	*/
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

/************************************************** CUBE_CREATE **************************************************/
/**
 * Creates a 2D size by size "cube" of pointers to nodes and returns it
 *
 * @param size 		Size of the sides of the cube
 * @return 			"Cube"
 */
struct node *** cube_create(int size)
{
	struct node 	***cube 		= NULL;
	struct node 	**cube_mem 		= NULL;
	int 			x 				= 0;

	cube = (struct node ***) calloc(size, sizeof(struct node **));
	cube_mem = (struct node **) calloc(size * size, sizeof(struct node *));
	alloc_check(cube);
	alloc_check(cube_mem);

	for(x = 0; x < size; x++)
	{
		cube[x] = &cube_mem[size * x];
	}

	return cube;
}

/************************************************** CUBE_DESTROY **************************************************/
/**
 * Frees the memory allocated for the structure required for the problem
 *
 * @param cube 		Structure that contains the cells
 * @param size 		Size of the sides of the cube
 */
void cube_destroy(struct node ***cube, int size)
{
	struct node 	*aux 	= NULL; 	/**< 	Auxilliary pointer 	*/
	int 			x 		= 0; 		/**< 	x-Coordinate 		*/
	int 			y 		= 0; 		/**< 	y-Coordinate 		*/

	for(x = 0; x < size; x++)
	{
		for(y = 0; y < size; y++)
		{
			while(cube[x][y] != NULL)
			{
				aux = cube[x][y];
				cube[x][y] = cube[x][y]->next;
				free(aux);
			}
		}
	}
	free(cube[0]);
	free(cube);
}

/************************************************** CUBE_PRINT **************************************************/
/**
 * Prints the solution of the problem to stdout
 *
 * @param cube 		Structure that contains the cells
 * @param size 		Size of the sides of the cube
 */
void cube_print(struct node ***cube, int size)
{
	struct node 	*aux 	= NULL; 	/**< 	Auxilliary pointer 	*/
	int 			x 		= 0; 		/**< 	x-Coordinate 		*/
	int 			y 		= 0; 		/**< 	y-Coordinate 		*/

	for(x = 0; x < size; x++)
	{
		for(y = 0; y < size; y++)
		{
			aux = cube[x][y];
			while(aux != NULL)
			{
				if(aux->status == ALIVE)
				{
					fprintf(stdout, "%d %d %d\n", x, y, aux->z);
				}
				aux = aux->next;
			}
		}
	}
}

/************************************************** CUBE_PURGE **************************************************/
/**
 * Clean up routine to remove dead cells from the already processed
 * structure as a way to speed up the next iteration
 *
 * @param cube 		Structure that contains the cells
 * @param size 		Size of the sides of the cube
 */
void cube_purge(struct node ***cube, int size)
{
	struct node 	**ptr 	= NULL; 	/**< 	Dereferencing pointer 	*/
	struct node 	*aux 	= NULL; 	/**< 	Auxilliary pointer 		*/
	int 			x 		= 0; 		/**< 	x-Coordinate 			*/
	int 			y 		= 0; 		/**< 	y-Coordinate 			*/

	for(x = 0; x < size; x++)
	{
		for(y = 0; y < size; y++)
		{
			ptr = &cube[x][y];
			aux = *ptr;
			while(aux != NULL)
			{
				if(aux->status == DEAD)
				{
					*ptr = aux->next;
					free(aux);
				}
				else
				{
					ptr = &aux->next;
				}
				aux = *ptr;
			}
		}
	}
}

/************************************************** DETERMINE_NEXT_GENERATION **************************************************/
/**
 * Iterates through all the cells in the cube and determines whether
 * they live or die in the next generation
 *
 * @param cube 		Structure that contains the cells
 * @param size 		Size of the sides of the cube
 */
void determine_next_generation(struct node ***cube, int size)
{
	struct node 	*aux 	= NULL; 	/**< 	Auxilliary pointer 	*/
	int 			x 		= 0; 		/**< 	x-Coordinate 		*/
	int 			y 		= 0; 		/**< 	y-Coordinate 		*/

	#pragma omp for schedule(static)
	for(x = 0; x < size; x++)
	{
		for(y = 0; y < size; y++)
		{
			aux = cube[x][y];
			while(aux != NULL)
			{
				/* If the cell is alive and has less than 2 or more than 4 neighbors it dies */
				if(aux->status == ALIVE)
				{
					if(aux->alive_neighbors < 2 || aux->alive_neighbors > 4)
					{
						aux->status = DEAD;
					}
				}
				/* If the cell is dead and has either 2 or 3 neighbors it comes to life */
				else
				{
					if(aux->alive_neighbors == 2 || aux->alive_neighbors == 3)
					{
						aux->status = ALIVE;
					}
				}
				/* Reset the number of neighbors of all processed cells */
				aux->alive_neighbors = 0;
				aux = aux->next;
			}
		}
	}
}

/************************************************** LOCKS_CREATE **************************************************/
/**
 * Creates and initializes a 2D size by size matrix of locks and returns it
 *
 * @param size 		Size of the sides of the matrix
 * @return 			Locks
 */
omp_lock_t ** locks_create(int size)
{
	omp_lock_t 		**locks 		= NULL;
	omp_lock_t 		*locks_mem 		= NULL;
	int 			x 				= 0;
	int 			y 				= 0;

	locks = (omp_lock_t **) calloc(size, sizeof(omp_lock_t *));
	locks_mem = (omp_lock_t *) calloc(size * size, sizeof(omp_lock_t));
	alloc_check(locks);
	alloc_check(locks_mem);

	for(x = 0; x < size; x++)
	{
		locks[x] = &locks_mem[size * x];
		for(y = 0; y < size; y++)
		{
			omp_init_lock(&(locks[x][y]));
		}
	}

	return locks;
}

/************************************************** LOCKS_DESTROY **************************************************/
/**
 * Destroys and frees the memory allocated for the locks required for the problem
 *
 * @param locks 	Structure that contains the locks
 * @param size 		Size of the sides of the matrix of locks
 */
void locks_destroy(omp_lock_t **locks, int size)
{
	int 			x 		= 0; 		/**< 	x-Coordinate 		*/
	int 			y 		= 0; 		/**< 	y-Coordinate 		*/

	for(x = 0; x < size; x++)
	{
		for(y = 0; y < size; y++)
		{
			omp_destroy_lock(&(locks[x][y]));
		}
	}
	free(locks[0]);
	free(locks);
}

/************************************************** MARK_NEIGHBORS **************************************************/
/**
 * Increments the alive neighbors count of all the neighbors of all
 * alive cells in the current generation
 *
 * @param locks		OpenMP locks to guarantee synchronization between all
 *					threads acessing the same list
 * @param cube 		Structure that contains the cells
 * @param size 		Size of the sides of the cube
 */
void mark_neighbors(omp_lock_t **locks, struct node ***cube, int size)
{
	struct node 	*aux 	= NULL; 	/**< 	Auxilliary pointer 													*/
	int 			tmp 	= 0; 		/**< 	Temporary variable to avoid computing the modulo multiple times 	*/
	int 			x 		= 0; 		/**< 	x-Coordinate 														*/
	int 			y 		= 0; 		/**< 	y-Coordinate 														*/
	int 			z 		= 0; 		/**< 	z-Coordinate 														*/

	#pragma omp for schedule(static)
	for(x = 0; x < size; x++)
	{
		for(y = 0; y < size; y++)
		{
			aux = cube[x][y];
			while(aux != NULL)
			{
				/* For every alive cell, add each of its 6 neighbors to the cube and/or increment their neighbor count */
				if(aux->status == ALIVE)
				{
					z = aux->z;
					/* Compute the modulo just once to avoid having to do it once for each of the following 3 operations */
					tmp = MOD((x+1), size);
					/* Set the lock corresponding to that list on the cube and unset it after the operation is done */
					omp_set_lock(&(locks[tmp][y]));
					add_node(&(cube[tmp][y]), NEIGHBOR, DEAD, z);
					omp_unset_lock(&(locks[tmp][y]));

					tmp = MOD((x-1), size);
					omp_set_lock(&(locks[tmp][y]));
					add_node(&(cube[tmp][y]), NEIGHBOR, DEAD, z);
					omp_unset_lock(&(locks[tmp][y]));

					tmp = MOD((y+1), size);
					omp_set_lock(&(locks[x][tmp]));
					add_node(&(cube[x][tmp]), NEIGHBOR, DEAD, z);
					omp_unset_lock(&(locks[x][tmp]));

					tmp = MOD((y-1), size);
					omp_set_lock(&(locks[x][tmp]));
					add_node(&(cube[x][tmp]), NEIGHBOR, DEAD, z);
					omp_unset_lock(&(locks[x][tmp]));

					tmp = MOD((z+1), size);
					omp_set_lock(&(locks[x][y]));
					add_node(&(cube[x][y]), NEIGHBOR, DEAD, tmp);
					omp_unset_lock(&(locks[x][y]));

					tmp = MOD((z-1), size);
					omp_set_lock(&(locks[x][y]));
					add_node(&(cube[x][y]), NEIGHBOR, DEAD, tmp);
					omp_unset_lock(&(locks[x][y]));
				}
				aux = aux->next;
			}
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
void node_add(struct node **head, short mode, short status, int z)
{
	struct node 	*aux 	= NULL; 	/**< 	Auxilliary pointer 		*/
	struct node 	*new 	= NULL; 	/**< 	Pointer to a new node 	*/

	/* Add the node at the start of the list if its either empty or if it has the smallest z-coordinate */
	if(((*head) == NULL) || ((*head)->z > z))
	{
		new = node_create(status, z);
		new->alive_neighbors += mode;
		new->next = (*head);
		(*head) = new;
	}
	/* If we're adding a neighbor and the z-coordinate matches one already existing, increment the neighbor count */
	else if((*head)->z == z)
	{
		(*head)->alive_neighbors += mode;
	}
	/* Remaining cases */
	else
	{
		aux = (*head);
		/* Go through the list until it either ends or we find a node with a greater z-coordinate */
		while((aux->next != NULL) && (aux->next->z <= z))
		{
			aux = aux->next;
		}
		/* Increment the neighbor count if the node we are inserting already exists */
		if(aux->z == z)
		{
			aux->alive_neighbors += mode;
		}
		/* Otherwise add the node */
		else
		{
			new = node_create(status, z);
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
 * @return 			Pointer to the cell created
 */
struct node * node_create(short status, int z)
{
	struct node 	*new 	= NULL; 	/**< 	Pointer to the new node 	*/

	new = (struct node *) calloc(1, sizeof(struct node));
	alloc_check(new);

	new->alive_neighbors = 0;
	new->status = status;
	new->z = z;
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
		fprintf(stderr, "Program is run with ./life3d [name-of-input-file] [number-of-iterations]\n");
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
 * Reads the input file and stores the given cells in the cube
 *
 * @param cube 				Structure that contains the cells
 * @param input_filename 	Name of the input file
 * @param size 				Size of the sides of the cube
 */
void read_coordinates(FILE *input_fd, struct node ***cube)
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
		node_add(&(cube[x][y]), NEW, ALIVE, z);
	}
}

/************************************************** READ_SIZE **************************************************/
/**
 * Reads the input file and returns the declared size of the sides of the cube
 *
 * @param input_fd 			File descriptor for the input file
 * @return size 			Size of the sides of the cube
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
	FILE 			*input_fd 			= NULL; 	/**< 	File descriptor for the input file 			*/
	omp_lock_t 		**locks 			= NULL; 	/**< 	OpenMP locks, one for each [x][y] pair 		*/
	struct node 	***cube 			= NULL; 	/**< 	Structure that contains the cells 			*/
	char 			*input_filename 	= NULL; 	/**< 	Name of the input file 						*/
	int 			iterations 			= 0; 		/**< 	Number of iterations to run the problem 	*/
	int 			size 				= 0; 		/**< 	Size of the sides of the cube 				*/

	/* Read the arguments given to the program */
	read_arguments(argc, argv, &input_filename, &iterations);
	/* Read the size of the problem */
	input_fd = fopen(input_filename, "r");
	size = read_size(input_fd);
	/* Create the data structure */
	cube = cube_create(size);
	/* Create the locks */
	locks = locks_create(size);
	/* Reads the input file and stores the given cells in the cube */
	read_coordinates(input_fd, cube);
	fclose(input_fd);

	/* Process the given problem */
	#pragma omp parallel
	{
		while(iterations > 0)
		{
			/* Mark the neighbors of the currently alive cells */
			mark_neighbors(locks, cube, size);
			/* Go over all the cells and check which ones are alive in the next generation */
			determine_next_generation(cube, size);
			/* Go over all the cells and remove the dead ones */
			cube_purge(cube, size);
			/* Make sure that only one of the threads decreases the number of iterations */
			#pragma omp single
			iterations--;
		}
	}

	/* Print the solution to stdout */
	cube_print(cube, size);
	/* Destroy the data structures */
	cube_destroy(cube, size);
	locks_destroy(locks, size);

	return 0;
}