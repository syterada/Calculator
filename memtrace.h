#ifndef __MEMTRACE_H__
#define __MEMTRACE_H__

// To turn on dynamic memory tracing comment tne next line or comment to turn off
// #define MEMTRACE

#ifdef MEMTRACE
#define malloc(size) memtrace_malloc(size) 
#define free(ptr) memtrace_free(ptr)

void *memtrace_malloc(size_t size); 
void memtrace_free(void *ptr);
void memtrace_stats(void);
#endif

#endif
