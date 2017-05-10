#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "pool.h"

/*
 This is simple I/O test client for pool.h that
 uses a very specific input format.

 Failure to follow format might lead to undefined behaviour.
 
 LET is a lowercase letter from a..z to identify each allocation

 c INT: create the pool of size INT
 d: destroy the current pool
 a LET INT: allocate memory to LET of size INT
 f LET: free memory associated with LET
 r LET INT: reallocate memory associated with LET to size INT
 m: print active
 n: print available
 q: quit
*/

// get_let() reads in a lowercase letter from input and returns
//   it as an index from 0...25
// effects: if unable to read in a letter, prints message and exits
int get_let(void) {
  char c = '\0';
  int result = scanf(" %c", &c);
  if (result != 1 || c < 'a' || c > 'z') {
    printf("exit: invalid letter\n");
    exit(1);
  }
  return c - 'a';
}

// get_int() reads in an int from input
// effects: if unable to read in a number, prints message and exits
int get_int(void) {
  int i;
  if (scanf("%d", &i) != 1) {
    printf("exit: invalid number\n");
    exit(1);
  }
  return i;
}

int main(void) {
  struct pool *p = NULL;
  char *letters[26] = {0};
  while (1) {
    char cmd;
    if (scanf(" %c", &cmd) != 1) break;
    if (cmd == 'q') break;
    if (cmd == 'c') {
      int size = get_int();
      if (p) {
        printf("create: fail (already created)\n");
      } else {
        p = pool_create(size);
      }
    } else if (cmd == 'd') {
      bool result = pool_destroy(p);
      if (result) {
        p = NULL;
      } else {
        printf("destroy: fail\n");
      }
    } else if (cmd == 'a') {
      int let = get_let();      
      int size = get_int();
      if (letters[let]) {
        printf("malloc %c %d: fail (already allocated)\n", 'a' + let, size);
      } else {
        char *ptr = pool_alloc(p, size);
        if (ptr) {
          letters[let] = ptr;
        } else {
          printf("malloc %c %d: fail\n", 'a' + let, size);
        }
      }
    } else if (cmd == 'f') {
      int let = get_let(); 
      if (pool_free(p, letters[let])) {
        letters[let] = NULL;
      } else {
        printf("free %c: fail\n", 'a' + let);
      }
    } else if (cmd == 'r') {
      int let = get_let();      
      int size = get_int();
      char *ptr = pool_realloc(p, letters[let], size);
      if (ptr) {
        letters[let] = ptr;
      } else {
        printf("realloc %c %d: fail\n", 'a' + let, size);
      } 
    } else if (cmd == 'm') {
      pool_print_active(p);
    } else if (cmd == 'n') {
      pool_print_available(p);
    } else {
      printf("Invalid command (%c).\n",cmd);
      break;
    }
  }
  if (p) {
    if (pool_destroy(p)) {
      printf("ERROR: did not destroy pool\n");
    } else {
      printf("ERROR: did not free all allocations\n");
    }
  }
}
