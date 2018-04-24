CC=gcc
CFLAGS=-Wall -g -lpthread

all: src/main.c
	$(CC) $(CFLAGS) -o out/mtwister.o -c src/mtwister.c
	$(CC) $(CFLAGS) -o out/rand.o -c src/rand.c 
	$(CC) $(CFLAGS) -o out/main src/main.c out/rand.o out/mtwister.o

clean:
	-rm -f out/*

