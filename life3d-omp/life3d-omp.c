/**
 *  \brief      OpenMP implementation of a 3D version of the Game of Life by John Conway
 *              for the Parallel and Distributed Computing course at IST 16/17 2nd Semester
 *              taught by Professor José Monteiro and Professor Luís Guerra e Silva
 *
 *  \author     Group #25
 *  \author     André Mendes        #66943
 *  \author     Nuno Venturinha     #67682
 *  \author     Daniel Sousa        #79129
 *  \version    1.0
 *  \date       07/04/2017
 *
 */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALIVE 1             /** \def Macro to differentiate alive from dead cells */
#define DEAD 0              /** \def Macro to differentiate alive from dead cells */
#define BUFFER_SIZE 200     /** \def Size of the file reading buffer */
#define NEIGHBOR 1          /** \def Macro to differentiate neighbor from new cells */
#define NEW 0               /** \def Macro to differentiate neighbor from new cells */

/** \struct
 * Structure that represents the cells used in the cube.
 * Holds the information related to a given cell and a pointer
 * to another one with the same x and y coordinates
 */
struct node
{
	short alive_neighbors;        /**< Number of alive neighbors this cell has */
	short status;                /**< Whether this cell is alive or dead */
	int z;                      /**< z-coordinate of this cell */
	struct node *next;          /**< Pointer to another cell with the same x and y coordinates */
};

/**
 * Modulo function instead of the C default that does not
 * give the desired output for negative numbers.
 * Useful to simplify the code when adding cells that exceed
 * the boundaries of the cube and wrap around to the other side
 *
 * @param a     Integer a
 * @param b     Integer b
 * @return      a mod b
 */
int mod(int a, int b)
{
	return (a < 0) ? (a % b + b) : (a % b);
}

/**
 * Creates the nodes that represent cells in the game.
 *
 * @param status    Status of the cell to create
 * @param z         Z-coordinate of the cell to create
 * @return          Pointer to the cell created
 */
struct node * create_node(short status, int z)
{
	struct node *new;

	if((new = (struct node *) malloc(sizeof(struct node))) == NULL)
	{
		fprintf(stderr, "Error with memory allocation\n");
		exit(0);
	}
	new->alive_neighbors = 0;
	new->status = status;
	new->z = z;
	new->next = NULL;

	return new;
}

/**
 * Adds a node to the given list representing the z-coordinate
 *
 * @param head      Head of the list where to insert the node
 * @param mode      A way to differentiate whether we're adding neighbors of alive cells or alive cells themselves.
 *                   It could be suppressed by duplicating code and creating a function for each case
 * @param status    Status of the cell to add
 * @param z         Z-coordinate of the cell to add
 */
void add_node(struct node **head, short mode, short status, int z)
{
    /** Add the node at the start of the list if its either empty or if it has the smallest z-coordinate */
	if(((*head) == NULL) || ((*head)->z > z))
    {
        struct node *new;

        new = create_node(status, z);
        new->alive_neighbors += mode;
        new->next = (*head);
        (*head) = new;
    }
    /**
     * In case we are adding a neighbor and the z-coordinate matches one already existing,
     * increment the neighbor count
     */
	else if((*head)->z == z)
	{
		(*head)->alive_neighbors += mode;
	}
    /** Remaining cases */
	else
	{
		struct node *aux;

		aux = (*head);
        /** Go through the list until it either ends or we find a node with a greater z-coordinate */
		while((aux->next != NULL) && (aux->next->z <= z))
		{
			aux = aux->next;
		}
        /** Increment the neighbor count if the node we are inserting already exists */
		if(aux->z == z)
		{
			aux->alive_neighbors += mode;
		}
        /** Otherwise add the node */
        else
        {
            struct node *new;

            new = create_node(status, z);
            new->alive_neighbors += mode;
            new->next = aux->next;
            aux->next = new;
        }
	}
}

/**
 * Iterates through all the cells in the cube and determines whether
 * they live or die in the next generation
 *
 * @param cube      The structure with the current generation
 * @param size      The size of the cube sides
 */
void determine_next_generation(struct node ***cube, int size)
{
	struct node *aux;
	int x, y;

	#pragma omp for collapse(2) schedule(static)
	for(x = 0; x < size; x++)
	{
		for(y = 0; y < size; y++)
		{
			aux = cube[x][y];
            /** Go through all the cells in this [x][y] coordinate and process them */
			while(aux != NULL)
			{
                /**
                 * If the cell is alive and has less than 2 or more than 4 neighbors it dies.
                 * Otherwise it stays alive for the next generation
                 */
				if(aux->status == ALIVE)
				{
					if(aux->alive_neighbors < 2 || aux->alive_neighbors > 4)
					{
						aux->status = DEAD;
					}
				}
                /**
                 * If the cell is dead and has either 2 or 3 neighbors it comes to life
                 * in the next generation. Otherwise it stays dead
                 */
				else
				{
					if(aux->alive_neighbors == 2 || aux->alive_neighbors == 3)
					{
						aux->status = ALIVE;
					}
				}
                /**
                 * Reset the number of neighbor of every cell so that it does not interfere with the next
                 * iteration
                 */
				aux->alive_neighbors = 0;
				aux = aux->next;
			}
		}
	}
}

/**
 * Increments the alive neighbors count of all the neighbors of all
 * alive cells in the current generation
 *
 * @param locks		OpenMP locks to guarantee synchronization between all
 *					threads acessing the same list
 * @param cube      The structure with the current generation
 * @param size      The size of the cube sides
 */
void mark_neighbors(omp_lock_t **locks, struct node ***cube, int size)
{
    struct node *aux;
    int tmp;
    int x, y, z;

    #pragma omp for collapse(2) schedule(static)
    for(x = 0; x < size; x++)
    {
        for(y = 0; y < size; y++)
        {
            aux = cube[x][y];
            /** Go through all the cells in this [x][y] coordinate and process them */
            while(aux != NULL)
            {
                /**
                 * For every alive cell, add each of its 6 neighbors to the cube.
                 * If they already exist it just increments their neighbor count
                 */
                if(aux->status == ALIVE)
                {
                    z = aux->z;

                    /** 
                     * Compute the modulo just once to avoid having to do it once for each of
                     * the following 3 operations.
                     * Set the lock corresponding to that list on the cube to avoid undefined
                     * behaviour due to concurrency and unset it after the operation is done
                     */
                    tmp = mod((x+1), size);
                    omp_set_lock(&(locks[tmp][y]));
                    add_node(&(cube[tmp][y]), NEIGHBOR, DEAD, z);
                    omp_unset_lock(&(locks[tmp][y]));

                    tmp = mod((x-1), size);
                    omp_set_lock(&(locks[tmp][y]));
                    add_node(&(cube[tmp][y]), NEIGHBOR, DEAD, z);
                    omp_unset_lock(&(locks[tmp][y]));

                    tmp = mod((y+1), size);
                    omp_set_lock(&(locks[x][tmp]));
                    add_node(&(cube[x][tmp]), NEIGHBOR, DEAD, z);
                    omp_unset_lock(&(locks[x][tmp]));

                    tmp = mod((y-1), size);
                    omp_set_lock(&(locks[x][tmp]));
                    add_node(&(cube[x][tmp]), NEIGHBOR, DEAD, z);
                    omp_unset_lock(&(locks[x][tmp]));

                    tmp = mod((z+1), size);
                    omp_set_lock(&(locks[x][y]));
                    add_node(&(cube[x][y]), NEIGHBOR, DEAD, tmp);
                    omp_unset_lock(&(locks[x][y]));

                    tmp = mod((z-1), size);
                    omp_set_lock(&(locks[x][y]));
                    add_node(&(cube[x][y]), NEIGHBOR, DEAD, tmp);
                    omp_unset_lock(&(locks[x][y]));
                }
                aux = aux->next;
            }
        }
    }
}

/**
 * Clean up routine to remove dead cells from the already processed
 * structure as a way to speed up the next iteration
 *
 * @param cube      The structure with the current generation
 * @param size      The size of the cube sides
 */
void purge(struct node ***cube, int size)
{
    struct node **head;
    struct node *curr;
    struct node *temp;
    int x, y;

    #pragma omp for collapse(2) schedule(static)
    for(x = 0; x < size; x++)
    {
        for(y = 0; y < size; y++)
        {
            head = &(cube[x][y]);
            /** Go through all the cells in this [x][y] coordinate and delete the dead ones */
            while(((*head) != NULL) && ((*head)->status == DEAD))
            {
                temp = (*head);
                (*head) = (*head)->next;
                free(temp);
            }
            for(curr = (*head); curr != NULL; curr = curr->next)
            {
                while((curr->next != NULL) && (curr->next->status == DEAD))
                {
                    temp = curr->next;
                    curr->next = temp->next;
                    free(temp);
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
	FILE *input_fd;             /**< File descriptor for the input file */

	omp_lock_t **locks;			/**< OpenMP locks, one for each [x][y] coordinate pair */
	struct node ***cube;        /**< Structure that contains the cells */
	struct node *aux;            /**< Auxiliary pointer */
	struct node *temp;            /**< Auxiliary pointer */

	char buffer[BUFFER_SIZE];    /**< Buffer to read the input file */
	char *input_filename;        /**< String to hold the name of the input file */

	int iterations;             /**< Number of iterations to run the problem */
	int size;                   /**< Size of the cube sides */
	int x, y, z;                /**< Auxiliary variables for storing read values and iterating the cube */

    /** Process the arguments given to the program */
    if(argc != 3)
    {
		fprintf(stderr, "Program is run with ./life3d-omp [name-of-input-file] [number-of-iterations]\n");
		exit(0);
    }
	if((input_filename = (char *) malloc((strlen(argv[1]) * sizeof(char)) + 1)) == NULL)
	{
		fprintf(stderr, "Error with memory allocation\n");
		exit(0);
	}
	strcpy(input_filename, argv[1]);
	iterations = atoi(argv[2]);
	if(iterations <= 0)
	{
		fprintf(stderr, "The number of iterations must be >= 1\n");
		exit(0);
	}

    /** Open the input file */
	if((input_fd = fopen(input_filename, "r")) == NULL)
	{
		fprintf(stderr, "Error opening given file\n");
		exit(0);
	}
	free(input_filename);

    /** Read just the first line which contains the size to be able to allocate the cube */
	fgets(buffer,BUFFER_SIZE,input_fd);
	if((sscanf(buffer,"%d", &size)) != 1)
	{
		fprintf(stderr, "Input file does not match specifications\n");
		exit(0);
	}

    /** Allocate the cube and the locks */
	if((cube = (struct node ***) malloc(size * sizeof(struct node **))) == NULL)
	{
		fprintf(stderr, "Error with memory allocation\n");
		exit(0);
	}
    if((locks = (omp_lock_t **) malloc(size * sizeof(omp_lock_t *))) == NULL)
	{
		fprintf(stderr, "Error with memory allocation\n");
		exit(0);
	}
    for(x = 0; x < size; x++)
    {
        if((cube[x] = (struct node **) malloc(size * sizeof(struct node *))) == NULL)
		{
			fprintf(stderr, "Error with memory allocation\n");
			exit(0);
		}
        if((locks[x] = (omp_lock_t *) malloc(size * sizeof(omp_lock_t))) == NULL)
		{
			fprintf(stderr, "Error with memory allocation\n");
			exit(0);
		}
        for(y = 0; y < size; y++)
        {
            cube[x][y] = NULL;
            omp_init_lock(&(locks[x][y]));
        }
    }

    /** Read the input file while adding the cells to the cube */
    while(fgets(buffer,BUFFER_SIZE,input_fd) != NULL)
    {
        if((sscanf(buffer,"%d %d %d", &x, &y, &z)) != 3)
		{
			fprintf(stderr, "Input file does not match specifications\n");
			exit(0);
		}
        add_node(&(cube[x][y]), NEW, ALIVE, z);
    }
    fclose(input_fd);

    /** Process the given problem */
    #pragma omp parallel
    {
        while(iterations > 0)
        {
            mark_neighbors(locks, cube, size);
            determine_next_generation(cube, size);
            purge(cube, size);
            /**
             * Make sure that only one of the threads decreases the number of iterations
             * or else some will be skipped
             */
            #pragma omp single
            iterations--;
        }
    }

    /** Print the solution to the standard output and frees the structure at the same time */
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
                temp = aux->next;
                free(aux);
                aux = temp;
            }
        }
        free(cube[x]);
    }
    free(cube);

    return 1;
}

