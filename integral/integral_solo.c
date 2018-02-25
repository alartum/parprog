#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    double S = 0;
    // Init borders of integration and step
    double left = 0, right = 1;
    // Number of parts if not given
    int N = 10e6;
    if (argc > 1){
    	N = atoi(argv[1]);
    }
   
    clock_t time_start = clock();

    double h = (right - left)/N;	
 
    S = trapezium(f, left, right, h);
    
    clock_t time_end = clock();
	
    double seconds = (double)(time_end - time_start) / CLOCKS_PER_SEC;
    printf("%ld %ld\n", time_start, time_end);
    printf("Non-parallel integral: %.16lf\n", S);
    printf("#%d Total non-parallel time: %.16lf\n", id, seconds);

    return 0;
}
