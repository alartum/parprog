#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <math.h>

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
    double* lims = 0;

	if (id == 0){
        begin = MPI_Wtime();
        
        lims = (double*)malloc(3*sizeof(double)*size);
        //Borders
        double left = 0, right = 1;
        // Number of parts if not given
        int N = 1e6;
        if (argc > 1){
            N = atoi(argv[1]);
        }
        printf("N=%d\n", N);
        double h = (right - left)/N;
        // Get quotient and reminder
        div_t d = div(N, size);
        
        int i = 0;
        for (i; i < d.rem; i++){
            lims[3*i] = left + h*i*(d.quot+1);
            lims[3*i+1] = left + lims[3*i] + h*(d.quot+1);
            lims[3*i+2] = h;
        }
        for (i; i < size; i++){
            lims[3*i] = left + h*(i*d.quot + d.rem);
            lims[3*i+1] = left + lims[3*i] + h*d.quot;
            lims[3*i+2] = h;
        }
    }
    else{
        lims = (double*)malloc(3*sizeof(double));
    }
    MPI_Scatter(lims, 3, MPI_DOUBLE, lims, 3, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // Accumulating partial summs
    double S_part = 0, S = 0;
    S_part = trapezium(f, lims[0], lims[1], lims[2]);
    printf("#%d [%lf; %lf] S = %.8lf\n", id, lims[0], lims[1], S_part);
    
    MPI_Reduce(&S_part, &S, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (id == 0){
        end = MPI_Wtime();
        printf("Parallel integral: %.16lf\n", S);
        total = end - begin;
        printf("#%d Total parallel time: %.16lf\n", id, total);
        //printf("%lf\n", total);
    }
    MPI_Finalize();
    if (id == 0){
        clock_t time_start = clock();

        lims[0] = 0;
        lims[1] = 1;
        double S_new;
        S_new = trapezium(f, lims[0], lims[1], lims[2]);
        clock_t time_end = clock();

        double seconds = (double)(time_end - time_start) / CLOCKS_PER_SEC;
        printf("Non-parallel integral: %.16lf\n", S_new);
        printf("Delta: %g\n", fabs(S_new - S));
        printf("#%d Total non-parallel time: %.16lf (clocks: %ld %ld)\n", id, seconds, time_start, time_end);
    }

    return 0;
}
