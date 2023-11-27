#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <sys/time.h>
#define SIZE 150
#define NUM_THREADS 4

int a[SIZE][SIZE], b[SIZE][SIZE];
atomic_int c[SIZE][SIZE];
struct timeval start, end;
typedef struct
{
    int start_row;
    int end_row;
} ThreadArg;

void *multiply(void *arg)
{
    ThreadArg *thread_arg = (ThreadArg *)arg;
    int start_row = thread_arg->start_row;
    int end_row = thread_arg->end_row;
    //printf("HEELO");
    for (int i = start_row; i < end_row; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            atomic_int sum = 0;
            for (int k = 0; k < SIZE; k++)
            {
                sum += a[i][k] * b[k][j];
                //printf("sum, %d\n", sum);
            }
            atomic_fetch_add(&c[i][j], sum);
        }
    }
    // for (int i = start_row; i < end_row; i++)
    // {
    //     for (int j = 0; j < SIZE; j++)
    //     {
    //         atomic_store(&c[i][j], 0);
    //     }
    // }
    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];
    ThreadArg thread_args[NUM_THREADS];
    srand(time(NULL));
    // Initialize matrices a and b with some values
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            a[i][j] = rand() % 10;
            b[i][j] = rand() % 10;
            atomic_init(&c[i][j], 0);

            //printf("a[i][j], %d\n", a[i][j]);
            //printf("B[i][j], %d\n", b[i][j]);
        }
    }

    int rows_per_thread = SIZE / NUM_THREADS;
    int remainder = SIZE % NUM_THREADS;
    gettimeofday(&start, NULL);
    
   
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i].start_row = i * rows_per_thread + (i < remainder ? i : remainder);
        thread_args[i].end_row = (i + 1) * rows_per_thread + (i + 1 < remainder ? i + 1 : remainder);
        pthread_create(&threads[i], NULL, multiply, &thread_args[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    long multiplication_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    double multiplication_time_seconds = multiplication_time / 1000000.0;
    printf("Multiplication (concurrent) took: %.6f seconds\n", multiplication_time_seconds);
    // Print the result matrix
    // for (int i = 0; i < SIZE; i++)
    // {
    //     for (int j = 0; j < SIZE; j++)
    //     {
    //         printf("%d ", atomic_load(&c[i][j]));
    //     }
    //     printf("\n");
    // }

    return 0;
}
