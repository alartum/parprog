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
double trapezium(double (*f)(double), long long from, long long to, double h){
    if (h <= 0 || from >= to)
        return 0;

    double S = 0;
   
    S += (f(h*from)+f(h*to))/2;
    for (long long i = from + 1; i < to - 1; i ++){
        S += f(h*i) + f(h*(i+1));
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
    long long* lims = 0;
    double h;
    long long N = 0;

    if (id == 0){ 
        lims = (long long*)malloc(2*sizeof(long long)*size);
        //Borders
        double left = 0, right = 1;
        // Number of parts if not given
        N = 1e6;
        if (argc > 1){
            N = atoi(argv[1]);
        }
        //printf("N=%d\n", N);
        begin = MPI_Wtime();
        h = (right - left)/N;
        // Get quotient and reminder
        div_t d = div(N, size);
        
        int i = 0;
        for (i; i < d.rem; i++){
            lims[2*i] = i*(d.quot+1);
            lims[2*i+1] = lims[2*i] + d.quot + 1;
        }
        for (i; i < size; i++){
            lims[2*i] = i*d.quot + d.rem;
            lims[2*i+1] = lims[2*i] + d.quot;
        }
    }
    else{
        lims = (long long*)malloc(2*sizeof(long long));
    }
    MPI_Scatter(lims, 2, MPI_LONG_LONG, lims, 2, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&h, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    // Accumulating partial summs
    double S_part = 0, S = 0;
    S_part = trapezium(f, lims[0], lims[1], h);
    printf("#%d [%lld; %lld] S = %.16lf\n", id, lims[0], lims[1], S_part);
    
    MPI_Reduce(&S_part, &S, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (id == 0){
        end = MPI_Wtime();
        printf("Parallel integral: %.16lf\n", S);
        total = end - begin;
        printf("#%d Total parallel time: %.16lf\n", id, total);
        //printf("%lf\n", total);
    } 
    if (id == 0){
        begin = MPI_Wtime();
        double S_new = 0;
        for (int i = 0; i < size; i++){
            double tmp = trapezium(f, lims[2*i], lims[2*i+1], h);
            printf("[S]#%d [%lld; %lld] S = %.16lf\n", i, lims[2*i], lims[2*i+1], tmp);
            S_new += tmp;
        }
        end = MPI_Wtime();
        total = end - begin;
        printf("Non-parallel integral: %.16lf\n", S_new);
        printf("Delta: %g\n", fabs(S_new - S));
        printf("#%d Total non-parallel time: %.16lf\n", id, total);
    }
 
    MPI_Finalize();
    
    return 0;
}
