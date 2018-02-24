CC=mpicc
CFLAGS=-std=c11 -O3
SOURCE=integral

all: prog 

clean:
	rm prog
prog: $(SOURCE).c
	$(CC) $(CFLAGS) $(SOURCE).c -o prog
