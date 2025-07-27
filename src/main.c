#include <assert.h>
#include <stddef.h>
#include <stdio.h>

typedef char byte;
#define M_SIZE 4096 
#define FR_SIZE 100

#define TODO(msg) assert(0 && msg);

#define F_FMT "(pos=%ld, size=%ld)"
#define F_ARG(f) ((byte *)(f).pos - mastack), (f).size
typedef struct {
  void *pos;
  size_t size;
} fr_blck;

static byte maheap[M_SIZE];
static size_t mp = 0;

static fr_blck free_blocks[FR_SIZE];
static size_t fp = 0;

void defrag_freeblks(void){
  TODO("implement defragmentation in freed blocks");
}

void *mamalloc_frblc(size_t n){
    fr_blck cur;
    void *to_ret = NULL;
    size_t to_rem = -1;

    for (int i ; i < fp; i++) {
      cur = free_blocks[i];    
      if (cur.size <= n){
        to_ret = cur.pos;   
        to_rem = i;
        break;
      }
    }

    if (to_rem >= 0) 
      for (int i = to_rem; i < fp - 1; i++) 
        free_blocks[i] = free_blocks[i+1];

    return to_ret; 
}

void *mamalloc(size_t n) {
  void *ret;

  if (n <= 0)
    return NULL;

  if (n + mp >= M_SIZE) { 
    ret = mamalloc_frblc(n); // try to fit in freed_blocks
    if (ret) {
      return ret;
    } else {
      defrag_freeblks();
      ret = mamalloc_frblc(n);

      if (!ret)
        printf("mamalloc: no space available for allocation");

      return ret;
    }
  }

  ret = &maheap[mp];
  mp += n;
  return ret;
}

void mafree(void *ptr, size_t size){
  assert(fp < FR_SIZE);

  fr_blck freed = {
    .pos = ptr,
    .size = size
  };

  free_blocks[fp++] = freed;
}

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

int main(){

  return 0;
}


/* ------------------------------TESTING------------------------------ */
int main2() {
  int *temp;
  for (int i = 0; i < 20; i++) {
    temp = mamalloc(4);
    *temp = i * 5;
    if (i % 5 == 0)
      mafree(temp, sizeof(int));
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
