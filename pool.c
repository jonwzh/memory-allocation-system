#include "pool.h"
#include <stdbool.h>
#include <stdio.h>

struct memory {
  int index;
  int len;
  struct memory *next;
};

struct pool {
  int size;
  int occupied;
  char *data;
  struct memory *m;
};

struct pool *pool_create(int size) {
  struct pool *result = malloc(sizeof(struct pool));
  result->size = size;
  result->occupied = 0;
  result->data = malloc(sizeof(char) * size);
  result->m = NULL;
  return result;
}

bool pool_destroy(struct pool *p) {
  if (p->m || p->occupied) return false;
  free(p->data);
  free(p);
  return true;
}

char *pool_alloc(struct pool *p, int size) {
  if (p->m == NULL) {
    if (p->size >= size) {
      p->m = malloc(sizeof(struct memory));
      p->m->index = 0;
      p->m->len = size;
      p->m->next = NULL;
      p->occupied += 1;
      return p->data;
    } else {
      return NULL;
    }
  }
  struct memory *curmem = p->m;
  struct memory *nextmem = p->m->next;
  for (int i = 0; i < p->occupied; ++i) {
    if (p->m->index != 0 && p->m->index > size) {
      struct memory *new = malloc(sizeof(struct memory));
      new->index = 0;
      new->len = size;
      new->next = curmem;
      p->m = new;
      return p->data;
    }
    if (nextmem) {
      if ((nextmem->index - (curmem->index + curmem->len)) >= size) {
        struct memory *new = malloc(sizeof(struct memory));
        new->index = curmem->index + curmem->len;
        new->len = size;
        new->next = curmem->next;
        curmem->next = new;
        p->occupied += 1;
        return p->data + curmem->index + curmem->len; //>?
      } else {
        curmem = nextmem;
        nextmem = nextmem->next;
        continue;
      }
    } else {
      if ((p->size - curmem->index - curmem->len) >= size) {
        struct memory *new = malloc(sizeof(struct memory));
        new->index = curmem->index + curmem->len;
        new->len = size;
        new->next = NULL;
        p->occupied += 1;
        curmem->next = new;
        break; // ??? size?  in the bottom
      } else {
        return NULL;
      }
    }
  }
  return p->data + curmem->index + curmem->len;
}

bool pool_free(struct pool *p, char *addr) {
  int free_index = addr - p->data;
  struct memory *curmem = p->m;
  struct memory *prevmem = NULL;
  if (curmem == NULL) {
    printf("1");
    return false;
  }
  while (curmem->index != free_index) {
    if (curmem == NULL) {
      printf("2");
      return false;
    }
    prevmem = curmem;
    curmem = curmem->next;
  }
  if (prevmem) {
    prevmem->next = curmem->next;
  } else {
    p->m = curmem->next; //!!!
  }
  free(curmem);
  p->occupied -= 1;
  return true;
}

// pool_realloc(p, addr, size) changes the size of the active allocation at
//   addr and returns the new address for the allocation.
//   returns NULL if addr does not correspond to an active allocation
//   or the pool can not be resized (in which case the original allocation
//   does not change)
// effects: modifies p if successful
// time: O(n) + O(k) where k is min(size, m) and
//       m is the size of the original allocation
char *pool_realloc(struct pool *p, char *addr, int size) {
  int realloc_index = addr - p->data;
  struct memory *curmem = p->m;
  struct memory *prevmem = NULL;
  while (curmem->index != realloc_index) {
    if (curmem == NULL) return NULL;
    prevmem = curmem;
    curmem = curmem->next;
  }
  if (curmem->len >= size) {
    curmem->len = size;
    return addr;
  } else {
    if (curmem->next) {
      if (curmem->next->index - curmem->index >= size) {
        curmem->len = size;
        return addr;
      } else {
        int original_len = curmem->len;
        int original_index = curmem->index;
        char *result = pool_alloc(p, size);
        if (result) {
          for (int i = 0; i < original_len; ++i) {
            result[i] = p->data[original_index + i];
          }
          if (prevmem) {
            prevmem->next = curmem->next;
          } else {
            p->m = curmem->next;
          }
          free(curmem);
        }
        return result;
      }
    } else if (p->size - curmem->index > size) {
      curmem->len = size;
      return addr;
    } else {
      return NULL;
    }
  }
}


// pool_print_active(p) prints out a description of the active allocations
//   in pool p using the following format:
//   "active: index1 [size1], index2 [size2], ..., indexN [sizeN]\n" or
//   "active: none\n" if there are no active allocations
//   where the index of an allocation is relative to the start of the pool
// effects: displays a message
// time: O(n)
void pool_print_active(struct pool *p) {
  printf("active: ");
  struct memory *curmem = p->m;
  int count = 0;
  while (curmem) {
    if (count) {
      printf(", ");
    }
    printf("%d [%d]", curmem->index, curmem->len);
    curmem = curmem->next;
    ++count;
  }
  if (count) {
    printf("\n");
  } else {
    printf("none\n");
  }
}

// pool_print_available(p) prints out a description of the available
//   contiguous blocks of memory still available in pool p:
//   "available: index1 [size1], index2 [size2], ..., indexM [sizeM]\n" or
//   "available: none\n" if all of the pool has been allocated
// effects: displays a message
// time: O(n)
void pool_print_available(struct pool *p) {
  printf("available: ");
  struct memory *curmem = p->m;
  struct memory *prevmem = NULL;
  if (curmem == NULL) {
    printf("0 [%d]\n", p->size);
    return;
  }
  int count = 0;
  while (curmem) {
    if (prevmem == NULL) {
      prevmem = curmem;
      curmem = curmem->next;
      continue;
    } else if (curmem->index - prevmem->index > prevmem->len) {
      if (count) {
        printf(", ");
      }
      printf("%d [%d]", prevmem->index + prevmem->len,
             curmem->index - prevmem->index - prevmem->len);
      ++count;
      prevmem = curmem;
      curmem = curmem->next;
    } else {
      prevmem = curmem;
      curmem = curmem->next;
    }
  }
  if (p->size - (prevmem->index + prevmem->len)) {
    if (count) {
      printf(", ");
    }
    printf("%d [%d]", prevmem->index + prevmem->len,
           p->size - (prevmem->index + prevmem->len));
    ++count;
  }
  if (count == 0) {
    printf("none\n");
  } else {
    printf("\n");
  }
}

