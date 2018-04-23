CC=gcc
CFLAGS=-std=gnu11 -O3
LDFLAGS=-lm -pthread -lrt

all: main

clean:
	rm *.out

main: main.c
	$(CC) $(CFLAGS) main.c $(LDFLAGS) -o main.out


