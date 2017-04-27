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
 * \date 		26/04/2017
 */
/************************************************** INCLUDE **************************************************/
#include <mpi.h>
#include <omp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/************************************************** DEFINE **************************************************/
#define ALIVE 								1		/** \def 	Macro to differentiate alive from dead cells 	*/
#define DEAD 								0		/** \def 	Macro to differentiate alive from dead cells 	*/
#define BUFFER_SIZE							200		/** \def 	Size of the file reading buffer 				*/
#define NEIGHBOR 							1		/** \def 	Macro to differentiate neighbor from new cells 	*/
#define NEW 								0		/** \def 	Macro to differentiate neighbor from new cells 	*/
#define ROOT 								0
#define BLOCK_LOW(rank,numprocs,size) 		((rank)*(size)/(numprocs))
#define BLOCK_HIGH(rank,numprocs,size) 		(BLOCK_LOW(((rank)+1),(numprocs),(size))-1)
#define BLOCK_SIZE(rank,numprocs,size) 		(BLOCK_HIGH((rank),(numprocs),(size))-BLOCK_LOW((rank),(numprocs),(size))+1)
#define BLOCK_OWNER(index,numprocs,size) 	(((numprocs) * ((index) + 1) - 1) / (size))
#define BORDER_LEFT 						1
#define BORDER_RIGHT 						((size_local)-2)
#define HALO_LEFT 							0
#define HALO_RIGHT 							((size_local)-1)
#define LEFT_PROC 							MOD((rank-1),(numprocs))
#define RIGHT_PROC 							MOD((rank+1),(numprocs))
#define MOD(a, b) 							(((a) < 0) ? ((a) % (b) + (b)) : ((a) % (b)))

/************************************************** PROTOTYPES **************************************************/
struct coordinates
{
	short x;
	short y;
	short z;
};

struct node
{
	short 				alive_neighbors;
	short 				status;
	struct coordinates 	coords;
	struct node 		*next;
};


void 					alloc_check 				(void *ptr);
void 					array_index_fix 			(struct coordinates *array, int *displs, int *recvcounts, int numprocs, int size_global);
struct node *** 		array_to_block 				(struct coordinates *array, int block_low, int cell_count, int size_global, int size_local);
void 					block_add_array 			(struct node ***block, struct coordinates *array, int cell_count, int index);
int 					block_count_alive 			(struct node ***block, int size_global, int size_local);
struct node *** 		block_create 				(int size_global, int size_local);
void 					block_destroy 				(struct node ***block, int size_global, int size_local);
void 					block_halo_purge 			(struct node ***block, int size_global, int type);
void 					block_index_fix 			(struct node ***block, int size_global, int size_local, int block_low);
void 					block_print 				(struct node ***block, int size_global, int size_local);
void 					block_purge 				(struct node ***block, int size_global, int size_local);
struct coordinates * 	block_to_array 				(struct node ***block, int *array_size, int size_global, int size_local);
void 					determine_next_generation 	(struct node ***block, int size_global, int size_local);
int * 					get_displs 					(int *counts, int numprocs);
int * 					get_recvcounts 				(int *cell_total, int cell_count, int numprocs);
int * 					get_sendcounts 				(struct coordinates *array, int cell_count, int numprocs, int size_global);
void 					mark_neighbors 				(struct node ***block, int size_global, int size_local);
void 					node_add 					(struct node **head, short mode, short status, short x, short y, short z);
struct node * 			node_create 				(short status, short x, short y, short z);
void 					read_arguments 				(int argc, char *argv[], char **input_filename, int *iterations);
void 					read_coordinates 			(FILE *input_fd, struct node ***block);
int 					read_size 					(FILE *input_fd);
int 					slice_count_alive 			(struct node **slice, int size_global);
struct coordinates * 	slice_to_array 				(struct node **slice, int *array_size, int size_global);


void alloc_check(void *ptr)
{
	if(ptr == NULL)
	{
		fprintf(stderr, "Error with memory allocation\n");
		abort();
	}
}

void array_index_fix(struct coordinates *array, int *displs, int *recvcounts, int numprocs, int size_global)
{
	int 	i 	= 0;
	int 	j 	= 0;

	for(i = 0; i < numprocs; i++)
	{
		for(j = 0; j < recvcounts[i]; j++)
		{
			array[displs[i] + j].x += (BLOCK_LOW(i, numprocs, size_global) - 2);
		}
	}
}

struct node *** array_to_block(struct coordinates *array, int block_low, int cell_count, int size_global, int size_local)
{
	struct node 	***block 	= NULL;
	int 			i 			= 0;
	int 			x 			= 0;
	int 			y 			= 0;
	int 			z 			= 0;

	block = block_create(size_global, size_local);

	for(i = 0; i < cell_count; i++)
	{
		x = array[i].x - block_low + 1;
		y = array[i].y;
		z = array[i].z;
		node_add(&(block[x][y]), NEW, ALIVE, x, y, z);
	}

	return block;
}


void block_add_array(struct node ***block, struct coordinates *array, int cell_count, int index)
{
	int 	i 	= 0;
	int 	x 	= 0;
	int 	y 	= 0;
	int 	z 	= 0;

	for(i = 0; i < cell_count; i++)
	{
		x = index;
		y = array[i].y;
		z = array[i].z;
		node_add(&(block[x][y]), NEW, ALIVE, x, y, z);
	}
}

int block_count_alive(struct node ***block, int size_global, int size_local)
{
	struct node 	*aux 	= NULL;
	int 			count 	= 0;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = 0; x < size_local; x++)
	{
		for(y = 0; y < size_global; y++)
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

struct node *** block_create(int size_global, int size_local)
{
	struct node 	***block 		= NULL;
	struct node 	**block_mem 	= NULL;
	int 			x 				= 0;

	block = (struct node ***) calloc(size_local, sizeof(struct node **));
	block_mem = (struct node **) calloc(size_local * size_global, sizeof(struct node *));
	alloc_check(block);
	alloc_check(block_mem);

	for(x = 0; x < size_local; x++)
	{
		block[x] = &block_mem[size_global * x];
	}

	return block;
}

void block_destroy(struct node ***block, int size_global, int size_local)
{
	struct node 	*aux 	= NULL;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = 0; x < size_local; x++)
	{
		for(y = 0; y < size_global; y++)
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


void block_halo_purge(struct node ***block, int size_global, int type)
{
	struct node 	*aux 	= NULL;
	int 			y 		= 0;

	for(y = 0; y < size_global; y++)
	{
		while(block[type][y] != NULL)
		{
			aux = block[type][y];
			block[type][y] = block[type][y]->next;
			free(aux);
		}
	}
}



void block_index_fix(struct node ***block, int size_global, int size_local, int block_low)
{
	struct node 	*aux 	= NULL;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = 0; x < size_local; x++)
	{
		for(y = 0; y < size_global; y++)
		{
			aux = block[x][y];
			while(aux != NULL)
			{
				if(aux->status == ALIVE)
				{
					aux->coords.x = aux->coords.x + block_low - 1;
				}
				aux = aux->next;
			}
		}
	}
}

void block_print(struct node ***block, int size_global, int size_local)
{
	struct node 	*aux 	= NULL;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = 0; x < size_local; x++)
	{
		for(y = 0; y < size_global; y++)
		{
			aux = block[x][y];
			while(aux != NULL)
			{
				if(aux->status == ALIVE)
				{
					fprintf(stdout, "%d %d %d\n", aux->coords.x, aux->coords.y, aux->coords.z);
				}
				aux = aux->next;
			}
		}
	}
}


void block_purge(struct node ***block, int size_global, int size_local)
{
	struct node 	**ptr 	= NULL;
	struct node 	*aux 	= NULL;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = BORDER_LEFT; x <= BORDER_RIGHT; x++)
	{
		for(y = 0; y < size_global; y++)
		{
			ptr = &block[x][y];
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

struct coordinates * block_to_array(struct node ***block, int *array_size, int size_global, int size_local)
{
	struct coordinates 	*array 	= NULL;
	struct coordinates 	*ptr 	= NULL;
	struct node 		*aux 	= NULL;
	int 				x 		= 0;
	int 				y 		= 0;

	(*array_size) = block_count_alive(block, size_global, size_local);

	array = (struct coordinates *) calloc((*array_size), sizeof(struct coordinates));
	alloc_check(array);
	ptr = array;

	for(x = 0; x < size_local; x++)
	{
		for(y = 0; y < size_global; y++)
		{
			aux = block[x][y];
			while(aux != NULL)
			{
				if(aux->status == ALIVE)
				{
					(*ptr).x = aux->coords.x;
					(*ptr).y = aux->coords.y;
					(*ptr).z = aux->coords.z;
					ptr++;
				}
				aux = aux->next;
			}
		}
	}

	return array;
}



void determine_next_generation(struct node ***block, int size_global, int size_local)
{
	struct node 	*aux 	= NULL;
	int 			x 		= 0;
	int 			y 		= 0;

	for(x = BORDER_LEFT; x <= BORDER_RIGHT; x++)
	{
		for(y = 0; y < size_global; y++)
		{
			aux = block[x][y];
			while(aux != NULL)
			{
				if(aux->status == ALIVE)
				{
					if(aux->alive_neighbors < 2 || aux->alive_neighbors > 4)
					{
						aux->status = DEAD;
					}
				}
				else
				{
					if(aux->alive_neighbors == 2 || aux->alive_neighbors == 3)
					{
						aux->status = ALIVE;
					}
				}
				aux->alive_neighbors = 0;
				aux = aux->next;
			}
		}
	}
}

int * get_displs(int *counts, int numprocs)
{
	int 	*displs 	= NULL;
	int 	i 			= 0;

	displs = (int *) calloc(numprocs, sizeof(int));
	alloc_check(displs);

	for(i = 1; i < numprocs; i++)
	{
		displs[i] = displs[i-1] + counts[i-1];
	}

	return displs;
}

int * get_recvcounts(int *cell_total, int cell_count, int numprocs)
{
	int 	*recvcounts 	= NULL;
	int 	i 				= 0;

	recvcounts = (int *) calloc(numprocs, sizeof(int));
	alloc_check(recvcounts);

	MPI_Gather(&cell_count, 1, MPI_INT, recvcounts, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	for(i = 0; i < numprocs; i++)
	{
		(*cell_total) += recvcounts[i];
	}

	return recvcounts;
}

int * get_sendcounts(struct coordinates *array, int cell_count, int numprocs, int size_global)
{
	int 	*sendcounts 	= NULL;
	int 	i 				= 0;

	sendcounts = (int *) calloc(numprocs, sizeof(int));
	alloc_check(sendcounts);

	for(i = 0; i < cell_count; i++)
	{
		sendcounts[BLOCK_OWNER(array[i].x, numprocs, size_global)]++;
	}

	return sendcounts;
}

void mark_neighbors(struct node ***block, int size_global, int size_local)
{
	struct node 	*aux 	= NULL;
	int 			x 		= 0;
	int 			y 		= 0;
	int 			z 		= 0;

	for(x = BORDER_LEFT; x <= BORDER_RIGHT; x++)
	{
		for(y = 0; y < size_global; y++)
		{
			aux = block[x][y];
			while(aux != NULL)
			{
				if(aux->status == ALIVE)
				{
					z = aux->coords.z;
					node_add(&(block[x+1][y]), NEIGHBOR, DEAD, x+1, y, z);
					node_add(&(block[x-1][y]), NEIGHBOR, DEAD, x-1, y, z);
					node_add(&(block[x][MOD((y+1), size_global)]), NEIGHBOR, DEAD, x, MOD((y+1), size_global), z);
					node_add(&(block[x][MOD((y-1), size_global)]), NEIGHBOR, DEAD, x, MOD((y-1), size_global), z);
					node_add(&(block[x][y]), NEIGHBOR, DEAD, x, y, MOD((z+1), size_global));
					node_add(&(block[x][y]), NEIGHBOR, DEAD, x, y, MOD((z-1), size_global));
				}
				aux = aux->next;
			}
		}
	}
	for(y = 0; y < size_global; y++)
	{
		aux = block[HALO_LEFT][y];
		while(aux != NULL)
		{
			if(aux->status == ALIVE)
			{
				node_add(&(block[BORDER_LEFT][y]), NEIGHBOR, DEAD, BORDER_LEFT, y, aux->coords.z);
			}
			aux = aux->next;
		}
	}
	for(y = 0; y < size_global; y++)
	{
		aux = block[HALO_RIGHT][y];
		while(aux != NULL)
		{
			if(aux->status == ALIVE)
			{
				node_add(&(block[BORDER_RIGHT][y]), NEIGHBOR, DEAD, BORDER_RIGHT, y, aux->coords.z);
			}
			aux = aux->next;
		}
	}
}

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
		(*head)->alive_neighbors += mode;
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
			aux->alive_neighbors += mode;
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

void read_coordinates(FILE *input_fd, struct node ***block)
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
		node_add(&(block[x][y]), NEW, ALIVE, x, y, z);
	}
}

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


int slice_count_alive(struct node **slice, int size_global)
{
	struct node 	*aux 	= NULL;
	int 			count 	= 0;
	int 			y 		= 0;

	for(y = 0; y < size_global; y++)
	{
		aux = slice[y];
		while(aux != NULL)
		{
			if(aux->status == ALIVE)
			{
				count++;
			}
			aux = aux->next;
		}
	}

	return count;
}

struct coordinates * slice_to_array(struct node **slice, int *array_size, int size_global)
{
	struct coordinates 	*array 	= 0;
	struct coordinates 	*ptr 	= 0;
	struct node 		*aux 	= 0;
	int 				y 		= 0;

	(*array_size) = slice_count_alive(slice, size_global);

	array = (struct coordinates *) calloc((*array_size), sizeof(struct coordinates));
	alloc_check(array);
	ptr = array;

	for(y = 0; y < size_global; y++)
	{
		aux = slice[y];
		while(aux != NULL)
		{
			if(aux->status == ALIVE)
			{
				(*ptr).x = aux->coords.x;
				(*ptr).y = aux->coords.y;
				(*ptr).z = aux->coords.z;
				ptr++;
			}
			aux = aux->next;
		}
	}

	return array;
}

/************************************************** MAIN **************************************************/
int main(int argc, char *argv[])
{
	MPI_Aint 			struct_displs[3] 	= {offsetof(struct coordinates, x), offsetof(struct coordinates, y), offsetof(struct coordinates, z)};
	MPI_Aint 			struct_extent;
	MPI_Aint 			struct_lb;
	MPI_Datatype 		MPI_COORDINATES;
	MPI_Datatype 		MPI_COORDINATES_t;
	MPI_Datatype 		struct_type[3] 		= {MPI_SHORT, MPI_SHORT, MPI_SHORT};
	MPI_Status 			status;
	FILE 				*input_fd 			= NULL;
	struct coordinates 	*receivebuffer 		= NULL;
	struct coordinates 	*sendbuffer 		= NULL;
	struct node 		***block_global 	= NULL;
	struct node 		***block_local 		= NULL;
	char 				*input_filename 	= NULL;
	int 				*displs 			= NULL;
	int 				*recvcounts 		= NULL;
	int 				*sendcounts 		= NULL;
	int 				struct_blocklen[3] 	= {1, 1, 1};
	int 				cell_count 			= 0;
	int 				cell_total 			= 0;
	int 				iterations 			= 0;
	int 				numprocs 			= 0;
	int 				rank 				= 0;
	int 				size_global 		= 0;
	int 				size_local 			= 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Type_create_struct(3, struct_blocklen, struct_displs, struct_type, &MPI_COORDINATES_t);
	MPI_Type_get_extent(MPI_COORDINATES_t, &struct_lb, &struct_extent);
	MPI_Type_create_resized(MPI_COORDINATES_t, struct_lb, struct_extent, &MPI_COORDINATES);
	MPI_Type_commit(&MPI_COORDINATES);

	if(rank == ROOT)
	{
		/* Check command line arguments */
		read_arguments(argc, argv, &input_filename, &iterations);
		/* Broadcast the number of iterations to all processes */
		MPI_Bcast(&iterations, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
		/* Open input file */
		input_fd = fopen(input_filename, "r");
		/* Read problem size */
		size_global = read_size(input_fd);
		/* Broadcast the total problem size to all processes */
		MPI_Bcast(&size_global, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
		/* Determine size for this process */
		size_local = BLOCK_SIZE(rank, numprocs, size_global) + 2;
		/* Allocate memory for the total problem size */
		block_global = block_create(size_global, size_global);
		/* Read coordinates from input file */
		read_coordinates(input_fd, block_global);
		fclose(input_fd);
		/* Convert the coordinates read into an array for sending */
		sendbuffer = block_to_array(block_global, &cell_count, size_global, size_global);
		block_destroy(block_global, size_global, size_global);
		/* Compute how many cells each process gets */
		sendcounts = get_sendcounts(sendbuffer, cell_count, numprocs, size_global);
		/* Compute the send buffer displacements */
		displs = get_displs(sendcounts, numprocs);
		/* Tell all processes how many cells to receive */
		MPI_Scatter(sendcounts, 1, MPI_INT, &cell_count, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
		/* Allocate memory to receive the assigned problem block */
		receivebuffer = (struct coordinates *) calloc(cell_count, sizeof(struct coordinates));
		/* Scatter the problem to all the processes */
		MPI_Scatterv(sendbuffer, sendcounts, displs, MPI_COORDINATES, receivebuffer, cell_count, MPI_COORDINATES, ROOT, MPI_COMM_WORLD);
		free(displs);
		free(sendbuffer);
		free(sendcounts);
		/* Turn the received array into a block for easier processing */
		block_local = array_to_block(receivebuffer, BLOCK_LOW(rank, numprocs, size_global), cell_count, size_global, size_local);
		free(receivebuffer);

		/* Problem solving loop */
		while(iterations > 0)
		{
			/* Turn the left border into an array */
			sendbuffer = slice_to_array(block_local[BORDER_LEFT], &cell_count, size_global);
			/* Send left border to left neighbor */
			MPI_Send(sendbuffer, cell_count, MPI_COORDINATES, LEFT_PROC, 0, MPI_COMM_WORLD);
			free(sendbuffer);
			/* Probe the sent message from the left neighbor */
			MPI_Probe(LEFT_PROC, 0, MPI_COMM_WORLD, &status);
			/* Get the size of the message */
			MPI_Get_count(&status, MPI_COORDINATES, &cell_count);
			/* Allocate memory to receive the left halo */
			receivebuffer = (struct coordinates *) calloc(cell_count, sizeof(struct coordinates));
			/* Receive the left halo */
			MPI_Recv(receivebuffer, cell_count, MPI_COORDINATES, LEFT_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			/* Add left halo to block */
			block_add_array(block_local, receivebuffer, cell_count, HALO_LEFT);
			free(receivebuffer);

			/* Turn the right border into an array */
			sendbuffer = slice_to_array(block_local[BORDER_RIGHT], &cell_count, size_global);
			/* Send right border to right neighbor */
			MPI_Send(sendbuffer, cell_count, MPI_COORDINATES, RIGHT_PROC, 0, MPI_COMM_WORLD);
			free(sendbuffer);
			/* Probe the sent message from the right neighbor */
			MPI_Probe(RIGHT_PROC, 0, MPI_COMM_WORLD, &status);
			/* Get the size of the message */
			MPI_Get_count(&status, MPI_COORDINATES, &cell_count);
			/* Allocate memory to receive the right halo */
			receivebuffer = (struct coordinates *) calloc(cell_count, sizeof(struct coordinates));
			/* Receive the right halo */
			MPI_Recv(receivebuffer, cell_count, MPI_COORDINATES, RIGHT_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			/* Add right halo to block */
			block_add_array(block_local, receivebuffer, cell_count, HALO_RIGHT);
			free(receivebuffer);

			mark_neighbors(block_local, size_global, size_local);
			determine_next_generation(block_local, size_global, size_local);
			block_purge(block_local, size_global, size_local);
			block_halo_purge(block_local, size_global, HALO_LEFT);
			block_halo_purge(block_local, size_global, HALO_RIGHT);

			iterations--;
		}
		
		sendbuffer = block_to_array(&(block_local[1]), &cell_count, size_global, BLOCK_SIZE(rank, numprocs, size_global));
		block_destroy(block_local, size_global, size_local);
		/* Get the number of elements to receive from each process */
		recvcounts = get_recvcounts(&cell_total, cell_count, numprocs);
		/* Compute the receive buffer displacements */
		displs = get_displs(recvcounts, numprocs);
		/* Allocate memory to receive the solution */
		receivebuffer = (struct coordinates *) calloc(cell_total, sizeof(struct coordinates));
		/* Receive all cells from all processes */
		MPI_Gatherv(sendbuffer, cell_count, MPI_COORDINATES, receivebuffer, recvcounts, displs, MPI_COORDINATES, ROOT, MPI_COMM_WORLD);
		free(sendbuffer);
		/* Restore the local indexes to the global indexes */
		array_index_fix(receivebuffer, displs, recvcounts, numprocs, size_global);
		free(displs);
		free(recvcounts);
		/* Create the block with the solution */
		block_global = array_to_block(receivebuffer, 0, cell_total, size_global, size_global);
		free(receivebuffer);
		/* Print the solution to stdout */
		block_print(block_global, size_global, size_global);
		block_destroy(block_global, size_global, size_global);
	}	
	else
	{
		/* Receive number of iterations */
		MPI_Bcast(&iterations, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
		/* Receive total problem size */
		MPI_Bcast(&size_global, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
		/* Determine size for this process */
		size_local = BLOCK_SIZE(rank, numprocs, size_global) + 2;
		/* Get how many cells to receive */
		MPI_Scatter(NULL, 0, NULL, &cell_count, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
		/* Allocate memory to receive the assigned problem block */
		receivebuffer = (struct coordinates *) calloc(cell_count, sizeof(struct coordinates));
		/* Receive the assigned problem block */
		MPI_Scatterv(NULL, NULL, NULL, NULL, receivebuffer, cell_count, MPI_COORDINATES, ROOT, MPI_COMM_WORLD);
		/* Turn the received array into a block for easier processing */
		block_local = array_to_block(receivebuffer, BLOCK_LOW(rank, numprocs, size_global), cell_count, size_global, size_local);
		free(receivebuffer);

		/* Problem solving loop */
		while(iterations > 0)
		{
			/* Probe the sent message from the right neighbor */
			MPI_Probe(RIGHT_PROC, 0, MPI_COMM_WORLD, &status);
			/* Get the size of the message */
			MPI_Get_count(&status, MPI_COORDINATES, &cell_count);
			/* Allocate memory to receive the right halo */
			receivebuffer = (struct coordinates *) calloc(cell_count, sizeof(struct coordinates));
			/* Receive the right halo */
			MPI_Recv(receivebuffer, cell_count, MPI_COORDINATES, RIGHT_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			/* Add right halo to block */
			block_add_array(block_local, receivebuffer, cell_count, HALO_RIGHT);
			free(receivebuffer);
			/* Turn the right border into an array */
			sendbuffer = slice_to_array(block_local[BORDER_RIGHT], &cell_count, size_global);
			/* Send right border to right neighbor */
			MPI_Send(sendbuffer, cell_count, MPI_COORDINATES, RIGHT_PROC, 0, MPI_COMM_WORLD);
			free(sendbuffer);

			/* Turn the left border into an array */
			sendbuffer = slice_to_array(block_local[BORDER_LEFT], &cell_count, size_global);
			/* Send left border to left neighbor */
			MPI_Send(sendbuffer, cell_count, MPI_COORDINATES, LEFT_PROC, 0, MPI_COMM_WORLD);
			free(sendbuffer);
			/* Probe the sent message from the left neighbor */
			MPI_Probe(LEFT_PROC, 0, MPI_COMM_WORLD, &status);
			/* Get the size of the message */
			MPI_Get_count(&status, MPI_COORDINATES, &cell_count);
			/* Allocate memory to receive the left halo */
			receivebuffer = (struct coordinates *) calloc(cell_count, sizeof(struct coordinates));
			/* Receive the left halo */
			MPI_Recv(receivebuffer, cell_count, MPI_COORDINATES, LEFT_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			/* Add left halo to block */
			block_add_array(block_local, receivebuffer, cell_count, HALO_LEFT);
			free(receivebuffer);

			mark_neighbors(block_local, size_global, size_local);
			determine_next_generation(block_local, size_global, size_local);
			block_purge(block_local, size_global, size_local);
			block_halo_purge(block_local, size_global, HALO_LEFT);
			block_halo_purge(block_local, size_global, HALO_RIGHT);

			iterations--;
		}

		/* Turn the local block into an array */
		sendbuffer = block_to_array(&(block_local[1]), &cell_count, size_global, BLOCK_SIZE(rank, numprocs, size_global));
		block_destroy(block_local, size_global, size_local);
		/* Tell ROOT how many elements to receive */
		MPI_Gather(&cell_count, 1, MPI_INT, NULL, 0, NULL, ROOT, MPI_COMM_WORLD);
		/* Send elements to ROOT */
		MPI_Gatherv(sendbuffer, cell_count, MPI_COORDINATES, NULL, NULL, NULL, NULL, ROOT, MPI_COMM_WORLD);
		free(sendbuffer);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}