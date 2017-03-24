OBJS = cube.o life3d.o node.o util.o
CC = gcc
DEBUG = -g
CFLAGS = -c -pedantic -Wall $(DEBUG)

default : life3d

life3d : $(OBJS)
		$(CC) $(LFLAGS) $(OBJS) -o life3d

life3d.o : life3d.c defs.h cube.h node.h util.h
	$(CC) $(CFLAGS) -c life3d.c

cube.o : cube.c cube.h defs.h util.h
	$(CC) $(CFLAGS) -c cube.c

node.o : node.c node.h
	$(CC) $(CFLAGS) -c node.c

util.o : util.c util.h defs.h node.h
	$(CC) $(CFLAGS) -c util.c

clean:
		$(RM) *.o *~ life3d