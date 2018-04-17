#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include <math.h>
#include <time.h>

typedef struct arg_t {
    int id;
    int nthreads;
    // Is data ready for the next thread
    // 0 = left border
    // 1 = right border
    sem_t* is_posted;
    // Is new data loaded by the next thread
    // 0 = left border
    // 1 = right border 
    sem_t* is_loaded;
    // Number of points being processed
    int npoints;
    // Storage arrays
    double* u;
    double* u_old;
    // Scheme constants
    double dt;
    double c;
    double h;
    double T;
    // Whether result is stored in u or u_old
    int swap_needed;
} arg_t;              

double g(double x){
    if (0 < x && x < 2)
        return x*(2-x);
    else
        return 0;
}

double u_new(double* u, double c, double h, double dt){
    double lambd = c*dt/h;
    return (1-lambd)*u[1] + lambd*u[0];
}

double u_precise(double c, double t, double x){
    return g(x - c*t);
}

int get_batch(int ndata, int nthreads, int id, int* offset){
    div_t d = div(ndata, nthreads);
    int npoints = 0;
    if (id < d.rem){
        npoints = d.quot + 1;
        *offset  = id*(d.quot+1);
    }
    else{
        npoints = d.quot;
        *offset = id*d.quot + d.rem;
    }

    return npoints;
}

void* thread_job(void* arg){
    // Unpacking locals
    arg_t* loc = (arg_t*)arg;
    int id = loc->id;
    int nthreads = loc->nthreads;
    sem_t* is_posted = loc->is_posted;
    sem_t* is_loaded = loc->is_loaded;
    int npoints = loc->npoints; 
    double* u = loc->u;
    double* u_old = loc->u_old;
    double dt = loc->dt;
    double c = loc->c;
    double h = loc->h;
    double T = loc->T;
    loc->swap_needed = 1;

    for (double t = 0; t < T + dt/2; t += dt){
        //printf("\n[%d] t = %lf\n", id, t);
        // If we are not the 1st thread
        
        for (int i = 1; i < npoints; i++){
            u[i] = u_new(&u_old[i-1], c, h, dt); 
        }
        if (id != nthreads-1){
            sem_post(is_posted+1);
        } 
        if (id != 0){
            sem_wait(is_posted);
           // printf("\n[%d] t = %lf, loading\n", id, t);
            u[0] = u_new(u_old-1, c, h, dt);
            sem_post(is_loaded);
        }
        else{
            u[0] = 0;
        }
        if (id != nthreads-1){
          //  printf("\n[%d] t = %lf, posted, waiting\n", id, t);     
            sem_wait(is_loaded+1);
        }
        double* tmp = u_old;
        u_old = u;
        u = tmp;
        // for (int i = 0; i < npoints; i++){
        //     printf("%g\n", u[i]);
        // }
        loc->swap_needed ^= loc->swap_needed; 
    }

    pthread_exit(NULL);
}

void* sync_thread_job(void* arg){
    // Unpacking locals
    arg_t* loc = (arg_t*)arg;
    int id = loc->id;
    int nthreads = loc->nthreads;
    sem_t* right = loc->is_posted;
    sem_t* left = loc->is_loaded;
    int npoints = loc->npoints; 
    double* u = loc->u;
    double* u_old = loc->u_old;
    double dt = loc->dt;
    double c = loc->c;
    double h = loc->h;
    double T = loc->T;
    loc->swap_needed = 1;

    for (double t = 0; t < T + dt/2; t += dt){
        //printf("\n[%d] t = %lf\n", id, t);
        // If we are not the 1st thread
        if (id == 0){
            u[0] = 0;
        }
        else{
            u[0] = u_new(u_old-1, c, h, dt);
        }   
            
        for (int i = 1; i < npoints; i++){
            u[i] = u_new(&u_old[i-1], c, h, dt); 
        }
        if (nthreads != 1){
            if (id == nthreads-1){
                sem_post(left);
                sem_wait(right);
            }
            else if (id == 0){
                sem_wait(left+1);
                sem_post(right+1);
            }
            else{
                sem_wait(left+1);
                sem_post(left);
                sem_wait(right);
                sem_post(right+1);
            }
        }

        double* tmp = u_old;
        u_old = u;
        u = tmp;
        // for (int i = 0; i < npoints; i++){
        //     printf("%g\n", u[i]);
        // }
        loc->swap_needed ^= loc->swap_needed; 
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    int nthreads = 4;
    if (argc > 1){
        nthreads = atoi(argv[1]);
    }
    double a = 0;
    double b = 10;
    int npoints = 100000;
    double h = (b-a)/npoints;

    double T = 4;
    double dt = h;//0.05;
    
    double c = 1;
    
    double* u = (double*)malloc(sizeof(double) * npoints);
    double* u_old = (double*)malloc(sizeof(double) * npoints);

    for (int i = 0; i < npoints; i++){
        u[i] = 0;
        u_old[i] = g(a + h*i);
    }

    sem_t* is_posted = (sem_t*)malloc(sizeof(sem_t) * (nthreads-1)); 
    sem_t* is_loaded = (sem_t*)malloc(sizeof(sem_t) * (nthreads-1));
    for (int i = 0; i < nthreads - 1; i++){
        sem_init(&is_posted[i], 0, 0);
        sem_init(&is_loaded[i], 0, 0);
    }

    pthread_t* thread_id = (pthread_t*)malloc(sizeof(pthread_t) * nthreads);
    arg_t* arg = (arg_t*)malloc(sizeof(arg_t) * nthreads);

    for (int i = 0; i < nthreads; i ++){
        arg[i].id = i;
        arg[i].nthreads = nthreads;
        // Careful handling of possible out-of-range
        arg[i].is_posted = &is_posted[i-1];
        arg[i].is_loaded = &is_loaded[i-1];
        int offset;
        arg[i].npoints = get_batch(npoints, nthreads, i, &offset);
        arg[i].u_old = &u_old[offset];
        arg[i].u = &u[offset];
        arg[i].c = c;
        arg[i].dt = dt;
        arg[i].h = h;
        arg[i].T = T;
    }

    struct timespec begin, end;
    double elapsed;
    clock_gettime(CLOCK_REALTIME, &begin);

    int rc;
    for (int i = 0; i < nthreads; i ++){
        rc = pthread_create(&thread_id[i], NULL, thread_job, &arg[i]);
        if (rc) printf("Error while creating: %d\n", rc);
    }

    for (int i = 0; i < nthreads; i++){
        rc = pthread_join(thread_id[i], NULL);
        if (rc) printf("Error while joining: %d\n", rc);
    }
    if (arg[0].swap_needed){
        double* tmp = u_old;
        u_old = u;
        u = tmp;
    }
    
    clock_gettime(CLOCK_REALTIME, &end);
    elapsed = end.tv_sec - begin.tv_sec;
    elapsed += (end.tv_nsec - begin.tv_nsec)/1000000000.0;

    printf("Time: %g s\n", elapsed);
    printf("Evaluation result:\n");
    int N = 10;
    for (int i = 0; i < N; i++){
        int n = npoints/N;
        double u_pr = u_precise(c, T + dt, a + h*i*n);
        double err = fabs(u[i*n] - u_pr);
        printf("x = %g  u = %g  u_pr = %g  err = %g\n", a + h*i*n, u[i*n], u_pr, err);
    }

    for (int i = 0; i < nthreads; i++){
        sem_destroy(&is_posted[i]);
        sem_destroy(&is_loaded[i]);
    }
    free(u);
    free(u_old);
    free(is_posted);
    free(is_loaded);
    free(thread_id);
    free(arg);

    return 0;
}
