CC=mpicc
CFLAGS=-std=c11 -O3
SOURCE=integral_verbose

all: $(SOURCE).out

clean:
	rm $(SOURCE).out
$(SOURCE).out: $(SOURCE).c
	$(CC) $(CFLAGS) $(SOURCE).c -o $(SOURCE).out
