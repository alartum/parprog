#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <math.h>

#define DUMMY_TAG 1
#define SQR(x) ((x)*(x))

#define M_PI 3.14159265358979323846
#define M_1_PI 0.318309886183790671538

double u_new(double* u, double k, double h, double dt){
    return u[1] + k*dt/SQR(h) * (u[0] - 2*u[1] + u[2]);
}

double u_precise(double k, double l, double t, double x, double eps){
    double u = 0;
    double du = 0;
    int m = 0;
    // Speed up
    if (x == 0.)
        return 0;
    double q1 = -t*SQR(M_PI/l)*k;
    double q2 = x/l*M_PI;
    double tmp = 2*eps;
    while (tmp > eps){
        tmp = exp(q1*SQR(2*m+1)) / (2*m+1);
        du = 4*M_1_PI * tmp * sin(q2*(2*m+1));
        m += 1;
        u += du;
    }
    return u;
}

void get_borders(int id, int size, int N, int* left, int* right){
    div_t d = div(N, size);
    
    // Splitting scheme:
    // [----|----|----]
    // - are calculation points, [] are borders
    // split only -
    if (id < d.rem){
        *left  = id*(d.quot+1);
        // +2 to correctly handle borders
        *right = *left + (d.quot+1) + 2;
    }
    else{
        *left = id*d.quot + d.rem;
        *right = *left + d.quot + 2;
    }
}

void send_borders(int id, int size, int n, double* u){
    // Right border 
    if (id != size - 1){
        // printf("#%d Sending u[%d]=%g to %d\n", id, n-2, u[n-2], id+1);
        MPI_Send(&u[n-2], 1, MPI_DOUBLE, id + 1, DUMMY_TAG, MPI_COMM_WORLD);
    }
    // Left border
    if (id != 0){
        // printf("#%d Sending u[%d]=%g to %d\n", id, 1, u[1], id-1);
        MPI_Send(&u[1], 1, MPI_DOUBLE, id - 1, DUMMY_TAG, MPI_COMM_WORLD);                
    }
}

void recv_borders(int id, int size, int n, double* u, MPI_Status* status){
    // Left border
    if (id != 0){
        MPI_Recv(&u[0], 1, MPI_DOUBLE, id - 1, DUMMY_TAG, MPI_COMM_WORLD, status);
        // printf("#%d Received u[%d]=%g from %d\n", id, 0, u[0], id-1);
    }
    else{
        u[0] = 0;
    }
    // Right border
    if (id != size - 1){
        MPI_Recv(&u[n-1], 1, MPI_DOUBLE, id + 1, DUMMY_TAG, MPI_COMM_WORLD, status);
        // printf("#%d Received u[%d]=%g from %d\n", id, n-1, u[n-1], id+1);
    }
    else{
        u[n-1] = 0;
    }
}

void array_print_debug(int id, int n, double t, double* u){
    char info[1000];
    int n_written = sprintf(info, "#%d [t=%g]", id, t);
    n_written += sprintf(&info[n_written], " (%g)", u[0]);
    for (int i = 1; i < n-1; i++){
        n_written += sprintf(&info[n_written], " [%g]", u[i]);
    }
    n_written += sprintf(&info[n_written], " (%g)", u[n-1]);
    sprintf(&info[n_written], "\n");
    printf(info);
}

void array_print(int id, int n, double t, double* u){
    char info[1000];
    int n_written = 0;
    n_written += sprintf(&info[n_written], "%g, ", u[0]);
    for (int i = 1; i < n-1; i++){
        n_written += sprintf(&info[n_written], "%g, ", u[i]);
    }
    n_written += sprintf(&info[n_written], "%g\n", u[n-1]);
    printf(info);
}

int main(int argc, char* argv[]){
	// Predefines
    double T = 1e-2;
    double l = 1;
    double k = 1;
    // Number of parts
    int N = 50;
    // Update N if provided
    if (argc > 1){
        N = atoi(argv[1]);
    }
    double h = l/N;
    double dt = 2e-4;
    
    if (dt >= h*h/k){
        fprintf(stderr, "Warning: Courant condition is not valid, changing dt: %g -> ", dt);
        dt = h*h/k/2;
        fprintf(stderr, "%g\n", dt);
    }
    
    int id, size;
    MPI_Status status;
	MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
    
    // Borders of computation area, right is not inclusive
    int left, right;
    // N-1 = number of split-points
    get_borders(id, size, N-1, &left, &right);
    // Size of segment for given process
    int n = right - left;

    double* u = (double*)malloc(n*sizeof(double));
    double* u_old = (double*)malloc(n*sizeof(double));
    
    for (int i = 0; i < n; i++){
        u_old[i] = 1;
    }
    // Border conditions
    if (id == 0) u_old[0] = 0;
    if (id == size-1) u_old[n-1] = 0;

    double t = 0;
    int nticks = 10000;
    int tick = 0;
    
    while (t < T + dt/2){
        // if (tick) array_print(id, n, t, u);
        // tick += 1;
        // tick %= nticks;
        
        t += dt;
        for (int i = 1; i < n-1; i++){
            u[i] = u_new(&u_old[i-1], k, h, dt);
        }
        // Update border values
        if (id % 2){
            send_borders(id, size, n, u);
            recv_borders(id, size, n, u, &status);
        }
        else{
            recv_borders(id, size, n, u, &status);
            send_borders(id, size, n, u);
        }
        // Swap the u and u_old
        double* tmp = u_old;
        u_old = u;
        u = tmp;
    }

    int m = (int)(0.1 / h);
    // printf("#%d m=%d left=%d right=%d h=%g\n", id, m, left, right, h);
    if (m){
        for (int i = left / m; i <= right / m; i++){
            if (i*m < right-1 && i*m > left || m*i == 0 || m*i == N){
                printf("#%d u(%g, %g) = %g; Delta = %g\n", id, h*i*m, T, u[i*m - left],
                                                    fabs(u[i*m - left] - u_precise(k, l, T, h*i*m, 1e-16)));
            }
        }
    }

    free(u);
    free(u_old);
    MPI_Finalize();
    // if (id == 0){
    //     for (double x = 0; x < 1.05; x += 0.1){
    //         printf("[PRECISE] u(%g, %g) = %.16lf\n", x, T, u_precise(k, l, T, x, 1e-16));
    //     }
    // }

    return 0;
}
