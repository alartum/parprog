CC=mpicc
CFLAGS=-std=c11 -O3
SOURCE=main_time

all: $(SOURCE).out

clean:
	rm $(SOURCE).out
$(SOURCE).out: $(SOURCE).c
	$(CC) $(CFLAGS) $(SOURCE).c -lm -o $(SOURCE).out
