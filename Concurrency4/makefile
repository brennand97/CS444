CC=gcc
CFLAGS=-Wall -g -lpthread

all: problem1 problem2

mtwister: src/mtwister.c
	$(CC) $(CFLAGS) -o out/mtwister.o -c src/mtwister.c

rand: src/rand.c
	$(CC) $(CFLAGS) -o out/rand.o -c src/rand.c

problem1: src/problem1.c rand mtwister
	$(CC) $(CFLAGS) -o out/problem1 src/problem1.c out/rand.o out/mtwister.o

problem2: src/problem2.c rand mtwister
	$(CC) $(CFLAGS) -o out/problem2 src/problem2.c out/rand.o out/mtwister.o

clean:
	-rm -f out/*

