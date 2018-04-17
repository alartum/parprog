#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include <math.h>
#include <time.h>

double HITS = 0;
#define TOTAL 100000000
sem_t sem;

typedef struct arg_t { 
    unsigned seed;
    int npoints; 
} arg_t;

void* monte_carlo(void* arg){
    arg_t* loc = (arg_t*)arg;
    double hits = 0;
    double x, y;

    for (int i = 0; i < loc->npoints; i++){
        x = (double)rand_r(&loc->seed)/RAND_MAX * M_PI;
        y = (double)rand_r(&loc->seed)/RAND_MAX;

        if (y < sin(x)) hits += x*y;
    }
    sem_wait(&sem);
    HITS += hits;
    sem_post(&sem);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    sem_init(&sem, 0, 1);
    
    int nthreads = 4;
    if (argc > 1){
        nthreads = atoi(argv[1]);
    }
    
    int rc;
    pthread_t* thread_id = (pthread_t*)malloc(sizeof(pthread_t) * nthreads);
    // First (TOTAL % nthreads) will get (TOTAL/nthreads + 1) points, other -- (TOTAL/nthreads)
    arg_t* arg = (arg_t*)malloc(sizeof(arg_t) * nthreads);
    div_t d = div(TOTAL, nthreads);
    struct timespec tmp;
    for (int i = 0; i < nthreads; i ++){
        arg[i].seed = i;
        arg[i].npoints = d.quot;
        if (i < d.rem) arg[i].npoints ++;
    }

    struct timespec begin, end;
    double elapsed;
    clock_gettime(CLOCK_REALTIME, &begin);

    for (int i = 0; i < nthreads; i ++){
        rc = pthread_create(&thread_id[i], NULL, monte_carlo, &arg[i]);
        if (rc) printf("Error while creating: %d\n", rc);
    }

    for (int i = 0; i < nthreads; i++){
        rc = pthread_join(thread_id[i], NULL);
        if (rc) printf("Error while joining: %d\n", rc);
    }
    
    clock_gettime(CLOCK_REALTIME, &end);
    elapsed = end.tv_sec - begin.tv_sec;
    elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;

    printf("Time: %g s\n", elapsed);
    printf("Integral: %g\n", M_PI*HITS/TOTAL);
    sem_destroy(&sem);
    return 0;
}
