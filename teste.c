#include <stdio.h>

#define MAMALLOC_DEBUG
#include "includes/mamalloc.h"

void show_status(void){
  printf("stack: ");
  print_stak();
  printf("freed: ");
  print_freed();
  printf("allocated: ");
  print_allocated();
  printf("\n");
}

int main(){
  int *a, *b, c;
  a = mamalloc(sizeof(int));
  b = mamalloc(sizeof(int));
  *a = 10;
  *b = 5;
  
  c = *a + *b;
  printf("c = %d\n", c);
  show_status();

  printf("\nfreeing b...\n");
  mafree(b);
  printf("after freeing b:\n");
  show_status();

  return 0;
}
