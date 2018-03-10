#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[]){
	int i;
	int array[10];
	int id, size;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	printf("I am %d of %d\n", id, size);
	
	if (id == 0){
		for (i; i < 10; i++){
			array[i] = i;
		}	
		MPI_Send(&array[5], 5, MPI_INT, 1, 1, MPI_COMM_WORLD);
	}
	if (id == 1){
		MPI_Recv(&array[5], 5, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
	} 
	
	MPI_Finalize();
	for (i = 0; i < 10; i++){
		printf("I am %d; array%d] = %d\n", id, i, array[i]);	
	}
		return 0;
}
