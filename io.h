#ifndef IO_H
#define IO_H

int read_input_size(FILE *input_fd);
void * read_input_coordinates(FILE *input_fd, struct coordinates *coordinates);

#endif