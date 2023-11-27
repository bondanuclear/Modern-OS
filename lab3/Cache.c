#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdatomic.h>
#include <pthread.h>
#define MATRIX_SIZE 150

//

//
void initializeMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            matrix[i][j] = rand() % 10;
            //printf(" initialize matrix %d\t", matrix[i][j]);
        }
    }
}

// cache-friendly
void multiplyMatrices(int result[MATRIX_SIZE][MATRIX_SIZE], int matrixA[MATRIX_SIZE][MATRIX_SIZE], int matrixB[MATRIX_SIZE][MATRIX_SIZE])
{
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            result[i][j] = 0;
            for (int k = 0; k < MATRIX_SIZE; k++)
            {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
    // for (int i = 0; i < MATRIX_SIZE; i++)
    // {
    //     for (int j = 0; j < MATRIX_SIZE; j++)
    //     {
    //         printf("%d\t", result[i][j]);
    //     }
    //     printf("\n");
    // }
}
// multiplying matrices in cache-unfriendly way
// using kij order
void multiplyMatricesKIJ(int result[MATRIX_SIZE][MATRIX_SIZE], int matrixA[MATRIX_SIZE][MATRIX_SIZE], int matrixB[MATRIX_SIZE][MATRIX_SIZE])
{
    for (int k = 0; k < MATRIX_SIZE; k++)
    {
        for (int i = 0; i < MATRIX_SIZE; i++)
        {
            for (int j = 0; j < MATRIX_SIZE; j++)
            {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
    // for (int i = 0; i < MATRIX_SIZE; i++)
    // {
    //     for (int j = 0; j < MATRIX_SIZE; j++)
    //     {
    //         printf("%d\t", result[i][j]);
    //     }
    //     printf("\n");
    // }
}
double measureExecutionTime(void (*matrixMultiplicationFunction)(int[MATRIX_SIZE][MATRIX_SIZE], int[MATRIX_SIZE][MATRIX_SIZE],
                                                                 int[MATRIX_SIZE][MATRIX_SIZE]),
                            int matrixA[MATRIX_SIZE][MATRIX_SIZE], int matrixB[MATRIX_SIZE][MATRIX_SIZE], int result[MATRIX_SIZE][MATRIX_SIZE])
{
    struct timeval startEval, endEval;
    gettimeofday(&startEval, NULL);
    matrixMultiplicationFunction(result, matrixA, matrixB);
    gettimeofday(&endEval, NULL);
    double elapsedTime = (endEval.tv_sec - startEval.tv_sec) + (endEval.tv_usec - startEval.tv_usec) / 1000000.0;
    return elapsedTime;
}

// random work with memory
// Function to shuffle an array of indices
void shuffleIndices(int indices[MATRIX_SIZE])
{
    for (int i = MATRIX_SIZE - 1; i > 0; i--)
    {    
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
        // printf("Indices i: %d\n", indices[i]);
        // printf("Indices j: %d\n", indices[j]);
    }
}

void multiplyMatricesRandom(int result[MATRIX_SIZE][MATRIX_SIZE], int matrixA[MATRIX_SIZE][MATRIX_SIZE], int matrixB[MATRIX_SIZE][MATRIX_SIZE])
{
    int randomIndices[MATRIX_SIZE];
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        randomIndices[i] = i;
    }
    shuffleIndices(randomIndices);
   
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            result[i][j] = 0;
            for (int k = 0; k < MATRIX_SIZE; k++)
            {
                result[i][j] += matrixA[i][randomIndices[k]] * matrixB[randomIndices[k]][j];
            }
        }
    }
}

//////
int main()
{
    struct timeval start, end;
   
    int matrixA[MATRIX_SIZE][MATRIX_SIZE];
    int matrixB[MATRIX_SIZE][MATRIX_SIZE];
    int result[MATRIX_SIZE][MATRIX_SIZE] = {0};
    srand(time(NULL));

    initializeMatrix(matrixA);
    initializeMatrix(matrixB);
    
    gettimeofday(&start, NULL);
    multiplyMatrices(result, matrixA, matrixB);
    gettimeofday(&end, NULL);
    long multiplication_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    double multiplication_time_seconds = multiplication_time / 1000000.0;
    printf("Multiplication (cache-friendly) took: %.6f seconds\n", multiplication_time_seconds);

    
    // random indices multiplication
    // gettimeofday(&start, NULL);
    // multiplyMatricesRandom(result, matrixA, matrixB);
    // gettimeofday(&end, NULL);
    // multiplication_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    // multiplication_time_seconds = multiplication_time / 1000000.0;
    // printf("Multiplication (cache-friednly) took: %.6f seconds\n", multiplication_time_seconds);

    // double executionTime = measureExecutionTime(multiplyMatricesRandom, matrixA, matrixB, result);
    // printf("Execution Time Random Indices: %f seconds\n", executionTime);
    // matrix multiplication (cache-unfriendly)
    // gettimeofday(&start, NULL);
    // multiplyMatricesKIJ(result, matrixA, matrixB);
    // gettimeofday(&end, NULL);
    // multiplication_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    // multiplication_time_seconds = multiplication_time / 1000000.0;
    // printf("Multiplication (cache-unfriednly) took: %.6f seconds\n", multiplication_time_seconds);


   

    return 0;
}
