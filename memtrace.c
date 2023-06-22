#include <stdio.h> 
#include <stdlib.h>
#include <malloc.h>
#include "uthash.h"

// This file implements a very simple dynamic memory
// tracing facility to catch and debug dynamic memory bugs

// structure
struct Memtrace_Alloc {
  void *addr;
  unsigned long long len;
  unsigned long long num;
  UT_hash_handle hh;
};

// hash table of allocations
static struct Memtrace_Alloc *memtrace_allocs = NULL;

// global private variables used to trace the number of times 
// and number of bytes malloced and freed
static long long memtrace_malloc_bytes = 0;
static long long memtrace_malloc_cnt = 0;
static long long memtrace_free_bytes = 0;
static long long memtrace_free_cnt = 0;
static long long memtrace_error_cnt = 0;
static int memtrace_init = 0;   // used to track if we have been called for the first time

// routine to dump that stats
void memtrace_stats(void)
{
  long long diff_cnt = memtrace_malloc_cnt - memtrace_free_cnt;
  long long diff_bytes = memtrace_malloc_bytes - memtrace_free_bytes;

  fflush(stdout);

  fprintf(stderr, 
	  "memtrace: SUMMARY: malloc:cnt=%lld bytes=%lld free:cnt=%lld bytes=%lld"
	  " diff:cnt:%lld bytes:%lld\n",
	  memtrace_malloc_cnt, memtrace_malloc_bytes, 
	  memtrace_free_cnt, memtrace_free_bytes, 
	  diff_cnt, diff_bytes);
  if (diff_cnt > 0) 
    fprintf(stderr, "memtrace: ERROR: %lld more mallocs that frees...leak???\n",
	    diff_cnt);
  if (diff_cnt < 0) 
    fprintf(stderr, "memtrace: ERROR: %lld more frees than mallocs...bug???\n",
	    diff_cnt);
  if (diff_bytes > 0)
    fprintf(stderr, "memtrace: ERROR: %lld more bytes malloced that frees...leak???\n", 
	    diff_bytes);
  if (diff_cnt < 0) 
    fprintf(stderr, "memtrace: ERROR: %lld more bytes freed than malloced...bug???\n", 
	    diff_bytes);
  if  (memtrace_error_cnt>0) 
    fprintf(stderr, "memtrace: ERROR: %lld memory errors\n", 
	    memtrace_error_cnt);
  if (memtrace_allocs != NULL) {
    struct Memtrace_Alloc *a;
    for (a=memtrace_allocs; a!=NULL; a=a->hh.next) {
      fprintf(stderr, "memtrace; ERROR: leaked: %lld: malloc(%lld)=%p\n",
	      a->num, a->len, a->addr);
    }
  }						  
}

/* malloc wrapper function  */ 
void *memtrace_malloc(size_t size) 
{
  if (!memtrace_init) {
    atexit(memtrace_stats);
    memtrace_init=1;
  }

  fflush(stdout);
  
  void *ptr = malloc(size);

  memtrace_malloc_cnt++;
  memtrace_malloc_bytes += size;

  struct Memtrace_Alloc *a; 
  HASH_FIND_PTR(memtrace_allocs, &ptr, a);
 
  if (a != NULL) {
    fprintf(stderr, "memtrace: ERROR: %p already alloced: len=%lld num=%lld\n",
	    a->addr, a->len, a->num);
    memtrace_error_cnt++;
  } else {
    a = (struct Memtrace_Alloc *)malloc(sizeof(struct Memtrace_Alloc));
    a->addr = ptr;
    a->len = size;
    a->num = memtrace_malloc_cnt;
    HASH_ADD_PTR(memtrace_allocs, addr, a);
  }

  fprintf(stderr, "memtrace: %lld: malloc(%d)=%p (bytes:%lld)\n", 
	  memtrace_malloc_cnt, (int)size,
	  ptr, memtrace_malloc_bytes);

  return ptr; 
}

/* free wrapper function */ 
void *memtrace_free(void *ptr) 
{
  long long len;
  long long num;

  fflush(stdout);

  free(ptr);
  memtrace_free_cnt++;

  struct Memtrace_Alloc *a; 
  HASH_FIND_PTR(memtrace_allocs, &ptr, a);
 
  if (a == NULL) {
    fprintf(stderr, "memtrace: ERROR: free of %p but no matching malloc!\n",
	    ptr);
    memtrace_error_cnt++;
  } else {
    len = a->len;
    num = a->num;
    HASH_DEL(memtrace_allocs, a);
    free(a);
    memtrace_free_bytes += len;
  }
  fprintf(stderr,  "memtrace: %lld: free(%p) (bytes:%lld) "
	  "-> %lld: malloc(%lld)=%p\n", 
	  memtrace_free_cnt, ptr, memtrace_free_bytes, num, len, ptr);
}

