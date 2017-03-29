CC = gcc
DEBUG = -ggdb
CFLAGS = $(DEBUG) -fopenmp -pedantic -Wall
OBJS = cube.o life3d-omp.o node.o util.o

default : life3d-omp

life3d-omp : $(OBJS)
	$(CC) $(CFLAGS) -o life3d-omp $(OBJS)

life3d-omp.o : life3d-omp.c defs.h cube.h node.h util.h
	$(CC) $(CFLAGS) -c life3d-omp.c

cube.o : cube.c cube.h defs.h node.h util.h
	$(CC) $(CFLAGS) -c cube.c

node.o : node.c node.h
	$(CC) $(CFLAGS) -c node.c

util.o : util.c util.h defs.h node.h
	$(CC) $(CFLAGS) -c util.c

clean:
		$(RM) *.o *~ life3d-omp