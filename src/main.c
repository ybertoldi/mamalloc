#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "../includes/mamalloc.h"

typedef char byte;
#define M_SIZE 4096 
#define FR_SIZE 100

#define TODO(msg) assert(0 && msg);
#define POS(ptr) ((long) ((byte*)ptr - maheap))

#define F_FMT "(pos=%ld, size=%ld)"
#define F_ARG(f) ((byte *)(f).pos - maheap), (f).size
typedef struct {
  void *pos;
  size_t size;
} fr_blck;

typedef struct{
  void* pos;
  size_t size;
} allocated_ptr;

static byte maheap[M_SIZE];
static size_t mp = 0;

static allocated_ptr mallocated[M_SIZE / 3];
static size_t ap = 0;

static fr_blck free_blocks[FR_SIZE];
static size_t fp = 0;


/* -------------------------DEBUG FUNCS------------------------- */
void print_stak(void){
  for (size_t i = 0; i < mp ; i += 4) {
    printf("%.4x ", *((int *)(&maheap[i])));    
  }
  printf("\n");
}

void print_freed(void){
  for (int i = 0; i < fp; i++) {
    printf(F_FMT" ", F_ARG(free_blocks[i]));
  }
  printf("\n");
}

void print_allocated(void){
  for (int i = 0; i < ap; i++) {
    printf("(pos=%ld, size=%ld)",
        (long)((byte *)mallocated[i].pos - maheap),
        mallocated[i].size);
  }
}
/* ------------------------------------------------------------- */

static void mallocated_add(void *ptr, size_t n){
  allocated_ptr to_add = {
    .pos = ptr,
    .size = n
  };
  mallocated[ap++] = to_add;
}

static allocated_ptr mallocated_get(void *ptr){
  allocated_ptr ret = {0};
  for (int i = 0; i < ap; i++) {
    if (mallocated[i].pos == ptr){
      ret = mallocated[i];
      break;
    }
  }

  return ret;
}

static void mallocated_del(void *ptr){
  int i, j;

  for (i = 0; i < ap; i++) 
    if (mallocated[i].pos == ptr) 
      break;

  if (i >= ap)
    return;

  for (j = i; i < ap - 1; j++) 
    mallocated[j] = mallocated[j+1];

  ap--;
}


static void del_frblk(int i){
  assert(i < fp);
  for (int j = i; j < fp - 1; j++) 
    free_blocks[j] = free_blocks[j+1];
  fp--;
}

static void defrag_freeblks(void) { // TODO: utilizar um algoritmo mais eficiente
  for (int i = 0; i < fp; i++) {
    for (int j = i; j < fp; j++) {
      if (free_blocks[i].pos + free_blocks[i].size == free_blocks[j].pos) {
        free_blocks[i].size += free_blocks[j].size;
        del_frblk(j);
      }
    }
  }
}

static void *mamalloc_frblc(size_t n){
    fr_blck cur;
    void *to_ret = NULL;
    int to_rem = -1;

    for (int i = 0 ; i < fp; i++) {
      cur = free_blocks[i];    
      if (cur.size >= n){
        to_ret = cur.pos;   

        if (cur.size == n){
          to_rem = i;
        } else {
          free_blocks[i].pos = cur.pos + n;
          free_blocks[i].size = cur.size - n;
        }
        break;
      }
    }

    if (to_rem >= 0)
      del_frblk(to_rem);

    return to_ret; 
}

static void _mafree(void *ptr, size_t size){
  assert(fp < FR_SIZE);

  fr_blck freed = {
    .pos = ptr,
    .size = size
  };

  free_blocks[fp++] = freed;
}

void *mamalloc(size_t n) {
  void *ret;

  if (n <= 0)
    return NULL;

  if (n + mp >= M_SIZE) { 
    ret = mamalloc_frblc(n); // try to fit in freed_blocks
    if (ret) {
      mallocated_add(ret, n);
      return ret;
    } else {
      defrag_freeblks();
      ret = mamalloc_frblc(n);

      if (!ret)
        printf("mamalloc: no space available for allocation");
      else
        mallocated_add(ret, n);


      return ret;
    }
  }

  ret = &maheap[mp];
  mp += n;
  mallocated_add(ret, n);
  return ret;
}

void mafree(void *ptr){
  allocated_ptr aptr = mallocated_get(ptr);  
  if (aptr.pos != 0 || aptr.size != 0){
    _mafree(aptr.pos, aptr.size);
    mallocated_del(ptr);
  }
}


/* ------------------------------TESTING------------------------------ */
int main3(){
  int *temp;
  for (int i = 0; i < 20; i++) {
    temp = mamalloc(4);
    *temp = i * 5;
    if (i % 5 == 0)
      mafree(temp);
  }
  printf("cur stack: \n");
  print_stak();

  printf("cur free blocks:\n");
  print_freed();

  printf("\nallocating memory in a free block\n");
  temp = mamalloc_frblc(sizeof(int));
  printf("index of new allocated int on the heap %ld\n", POS(temp));
  *temp = 420;

  printf("\nallocating memory in a free block\n");
  char * temp_c = mamalloc_frblc(sizeof(char));
  printf("index of new allocated char on the heap %ld\n", (long) (temp_c - maheap));
  *temp_c = 'a';

  printf("cur stack: \n");
  print_stak();

  printf("cur free blocks:\n");
  print_freed();

  printf("\nfreeing block at pos 24 and defragging\n");
  _mafree(&maheap[24], 4);
  defrag_freeblks();
  printf("cur free blocks:\n");
  print_freed();
  return 0;
}


int main2() {
  int *temp;
  for (int i = 0; i < 20; i++) {
    temp = mamalloc(4);
    *temp = i * 5;
    if (i % 5 == 0)
      _mafree(temp, sizeof(int));
  }
  printf("cur stack: \n");
  print_stak();

  printf("cur free blocks:\n");
  print_freed();

  //zero out freed_blocks
  for (int i = 0; i < fp; i++) {
    byte *ptr = free_blocks[i].pos;
    size_t len = free_blocks[i].size;

    while (len) {
      *(ptr + len - 1) = 0;
      len--;
    }
  }

  printf("stack after zeroing freed blocks:\n");
  print_stak();

  return 0;
}

int main1() {
  int *a, *b, c;
  a = mamalloc(sizeof(int));
  b = mamalloc(sizeof(int));
  *a = 10;
  *b = 5;
  c = *a + *b;

  printf("c = %d\n", c);
  printf("distance from b to the start of the array: %ld\n",
         ((byte *)b - maheap));

  printf("cur stack: \n");
  print_stak();
  return 0;
}
