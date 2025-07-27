#include <stddef.h>

#ifndef MAMALLOC_H
#define MAMALLOC_H 

void *mamalloc(size_t n);
void mafree(void *ptr);
#ifdef MAMALLOC_DEBUG
void print_stak(void);
void print_freed(void);
void print_allocated(void);
#endif

#endif

