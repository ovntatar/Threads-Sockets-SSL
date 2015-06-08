CC = gcc
CFLAGS = -Wall

BINARIES = average-mutex-cv average-mutex average pthreads-exit1 \
  pthreads-exit2 pthreads-lifecycle pthreads-signal

all: $(BINARIES)

average-mutex-cv: average-mutex-cv.o
	$(CC) -o $@ $@.o -lpthread

average-mutex: average-mutex.o
	$(CC) -o $@ $@.o -lpthread

average: average.o
	$(CC) -o $@ $@.o -lpthread

pthreads-exit1: pthreads-exit1.o
	$(CC) -o $@ $@.o -lpthread

pthreads-exit2: pthreads-exit2.o
	$(CC) -o $@ $@.o -lpthread

pthreads-lifecycle: pthreads-lifecycle.o
	$(CC) -o $@ $@.o -lpthread

pthreads-signal: pthreads-signal.o
	$(CC) -o $@ $@.o -lpthread

clean:
	rm -f $(BINARIES) *.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c
