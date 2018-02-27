CC=mpicc
CFLAGS=-std=c11 -O3
SOURCE=better_integral

all: $(SOURCE).out

clean:
	rm $(SOURCE).out
$(SOURCE).out: $(SOURCE).c
	$(CC) $(CFLAGS) $(SOURCE).c -o $(SOURCE).out
