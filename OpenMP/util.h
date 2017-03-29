#ifndef UTIL_H
#define UTIL_H

#include "node.h"

omp_lock_t ** create_locks(int size);
int mod(int a, int b);
void write_output(struct node ***cube, int size);

#endif