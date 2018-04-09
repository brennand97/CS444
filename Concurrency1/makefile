CC=gcc
CFLAGS=-Wall -g -lpthread

all: src/main.c src/buffer.c
	$(CC) $(CFLAGS) -o out/buffer.o -c src/buffer.c
	$(CC) $(CFLAGS) -o out/main.o -c src/main.c
	$(CC) $(CFLAGS) -o out/main out/buffer.o out/main.o

clean:
	-rm -f out/*

