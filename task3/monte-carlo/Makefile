CC=gcc
CFLAGS=-std=gnu11 -O3
LDFLAGS=-lm -lpthread -lrt

all: main better_main

clean:
	rm *.out

main: main.c
	$(CC) $(CFLAGS) main.c $(LDFLAGS) -o main.out

better_main: better_main.c
	$(CC) $(CFLAGS) better_main.c $(LDFLAGS) -o better_main.out




