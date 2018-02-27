#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define DUMMY_TAG 1

double f(double x){
    return 4/(1 + x*x);
}

// Returns integral 'from' -> 'to' of 'f' by trapezium rule with step 'h'
double trapezium(double (*f)(double), double from, double to, double h){
    if (h <= 0 || from >= to)
        return 0;

    double S = 0;
    double x = from + h;

    S += (f(from) + f(to))/2;
    while (x + h/2 < to){
        S += f(x);
        x += h;
    }

    return S*h;
}

int main(int argc, char* argv[]){
    int id, size;
    MPI_Status status;
    MPI_Init(&argc, &argv);

    double begin, end, total;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // Init borders of integration and step
    double lims[3];

    if (id == size - 1){
        begin = MPI_Wtime();
        
        //Borders
        double left = 0, right = 1;
        // Number of parts if not given
        int N = 1e6;
        if (argc > 1){
            N = atoi(argv[1]);
        }
        //printf("N = %d\n", N);

        double h = (right - left)/N;
        lims[2] = h;
        // Get quotient and reminder
        div_t d = div(N, size);
        
        int i = 0;
        for (i; i < d.rem; i++){
            lims[0] = left + h*i*(d.quot+1);
            lims[1] = left + lims[0] + h*(d.quot+1);
            MPI_Send(lims, 3, MPI_DOUBLE, i, DUMMY_TAG, MPI_COMM_WORLD);
        }
        for (i; i < size - 1; i++){
            lims[0] = left + h*(i*d.quot + d.rem);
            lims[1] = left + lims[0] + h*d.quot;
            MPI_Send(lims, 3, MPI_DOUBLE, i, DUMMY_TAG, MPI_COMM_WORLD);
        }

        lims[0] = right - h*d.quot;
        lims[1] = right;
    }
    else{
        MPI_Recv(lims, 3, MPI_DOUBLE, size - 1, DUMMY_TAG, MPI_COMM_WORLD, &status);
    }

    // Accumulating partial summs
    double S = 0;
    S = trapezium(f, lims[0], lims[1], lims[2]);
    //printf("#%d [%lf; %lf] S = %.8lf\n", id, lims[0], lims[1], S);

    if (id == size - 1){
        double temp;
        for (int i = 0; i < size - 1; i++){
            MPI_Recv(&temp, 1, MPI_DOUBLE, i, DUMMY_TAG, MPI_COMM_WORLD, &status);
            S += temp;
        }

        //printf("Parallel integral: %.16lf\n", S);

        end = MPI_Wtime();
        total = end - begin;
        //printf("#%d Total parallel time: %lf\n", id, total);
        printf("%lf\n", total);
    }
    else{
        MPI_Send(&S, 1, MPI_DOUBLE, size - 1, DUMMY_TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    if (id == size-1){
        clock_t time_start = clock();

        lims[0] = 0;
        lims[1] = 1;
        S = trapezium(f, lims[0], lims[1], lims[2]);
        clock_t time_end = clock();
        double seconds = (double)(time_end - time_start) / CLOCKS_PER_SEC;
        //printf("Non-parallel integral: %.16lf\n", S);
        //printf("#%d Total non-parallel time: %lf\n", id, seconds);
    }

    return 0;
}
