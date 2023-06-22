.PHONY: all clean

all: assignment-5 assignment-5_memtrace

memtrace.o: memtrace.c memtrace.h
	${CC} -std=c99 -g -D MEMTRACE -c memtrace.c

stackADT4.o: stackADT.h stackADT4.c memtrace.h 
	${CC} -std=c99 -g -c stackADT4.c

stackADT4_memtrace.o: stackADT.h stackADT4.c memtrace.h 
	${CC} -std=c99 -g -D MEMTRACE -c stackADT4.c -o stackADT4_memtrace.o

assignment-5: assignment-5.c stackADT.h memtrace.h stackADT4.o memtrace.o
	${CC} -std=c99 -g assignment-5.c stackADT4.o memtrace.o -o assignment-5

assignment-5_memtrace: assignment-5.c stackADT.h memtrace.h stackADT4_memtrace.o memtrace.o
	${CC} -std=c99 -g -D MEMTRACE assignment-5.c stackADT4_memtrace.o memtrace.o -o assignment-5_memtrace

clean:
	-${RM} assignment-5 assignment-5_memtrace *.o *.out
