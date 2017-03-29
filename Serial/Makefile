CC = gcc
DEBUG = -ggdb
CFLAGS = $(DEBUG) -pedantic -Wall
OBJS = cube.o life3d.o node.o util.o

default : life3d

life3d : $(OBJS)
	$(CC) $(CFLAGS) -o life3d $(OBJS)

life3d.o : life3d.c defs.h cube.h node.h util.h
	$(CC) $(CFLAGS) -c life3d.c

cube.o : cube.c cube.h defs.h node.h util.h
	$(CC) $(CFLAGS) -c cube.c

node.o : node.c node.h
	$(CC) $(CFLAGS) -c node.c

util.o : util.c util.h defs.h node.h
	$(CC) $(CFLAGS) -c util.c

clean:
		$(RM) *.o *~ life3d