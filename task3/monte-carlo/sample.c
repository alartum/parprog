#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 10

int sum = 0;
sem_t sem;

void* start_func(void* param){
    int* local = (int*)param;
    sem_wait(&sem);
    for (int i = 0; i < 10000; i++){
                *local = *local + 1; 
    }
    sem_post(&sem);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    sem_init(&sem, 0, 1);
    
    int rc;
    pthread_t pthr[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++){
        rc = pthread_create(&pthr[i], NULL, start_func, &sum);
        if (rc) printf("Error while creating: %d\n", rc);
    }
    for (int i = 0; i < NUM_THREADS; i++){
        rc = pthread_join(pthr[i], NULL);
        if (rc) printf("Error while joining: %d\n", rc);
    }
    
    printf("Sum is %d\n", sum);
    sem_destroy(&sem);
    return 0;
}
