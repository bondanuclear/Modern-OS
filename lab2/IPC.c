#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include "mmap_latency.h"

#define SHARED_MEMORY_SIZE 1024
#define DATA_SIZE 100000
#define FILE_PATH "communication_file.txt"
char generateRandomLetter()
{
    return (rand() % 26) + 'A';
}

// prot write latency
void measure_write_latency()
{
    struct timeval start, end;
    gettimeofday(&start, NULL);

    int shared_memory_fd = shm_open("/my_shared_memory_write", O_CREAT | O_RDWR, 0666);
    if (shared_memory_fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shared_memory_fd, SHARED_MEMORY_SIZE) == -1)
    {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    void *shared_memory = mmap(0, SHARED_MEMORY_SIZE, PROT_WRITE, MAP_PRIVATE, shared_memory_fd, 0);
    if (shared_memory == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    gettimeofday(&end, NULL);

    long connection_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    double connection_time_seconds = connection_time / 1000000.0;
    printf("Latency for PROT_WRITE mode: %.6f seconds\n", connection_time_seconds);

    munmap(shared_memory, SHARED_MEMORY_SIZE);
    close(shared_memory_fd);

    shm_unlink("/my_shared_memory_write");
}

// latency for prot read
void measure_read_latency()
{
    struct timeval start, end;
    gettimeofday(&start, NULL);

    int shared_memory_fd = shm_open("/my_shared_memory_read", O_CREAT | O_RDWR, 0666);
    if (shared_memory_fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shared_memory_fd, SHARED_MEMORY_SIZE) == -1)
    {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    void *shared_memory = mmap(0, SHARED_MEMORY_SIZE, PROT_READ, MAP_PRIVATE, shared_memory_fd, 0);
    if (shared_memory == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    gettimeofday(&end, NULL);

    long connection_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    double connection_time_seconds = connection_time / 1000000.0;
    printf("Latency for PROT_READ mode: %.6f seconds\n", connection_time_seconds);

    munmap(shared_memory, SHARED_MEMORY_SIZE);
    close(shared_memory_fd);

    shm_unlink("/my_shared_memory_read");
}

int main()
{
     file_based_measurement();
    //mmap_throughput();
    return 0;
}
void file_based_measurement()
{
    srand(time(NULL));

    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    struct timeval start, end;
    if (child_pid == 0)
    {
        gettimeofday(&start, NULL);
        FILE *file_write = fopen(FILE_PATH, "w");
        if (file_write == NULL)
        {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        gettimeofday(&end, NULL);

        long connection_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
        // convert to seconds
        double connection_time_seconds = connection_time / 1000000.0;
        printf("Latency before connecting to the file: %.6f seconds\n", connection_time_seconds);

       
        for (int i = 0; i < DATA_SIZE; i++)
        {
            fprintf(file_write, "%c", generateRandomLetter());
        }
        fclose(file_write);
    }
    else
    {
    
        wait(NULL);

        FILE *file_read = fopen(FILE_PATH, "r");
        if (file_read == NULL)
        {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        gettimeofday(&start, NULL);
    
        char buffer[DATA_SIZE + 1];
        fscanf(file_read, "%s", buffer);

       
        fclose(file_read);
        gettimeofday(&end, NULL);

        long read_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
        // convert to seconds
        double read_time_seconds = read_time / 1000000.0;
        printf("Throughput time of reading from file: %.6f seconds\n", read_time_seconds);
        long throughput = DATA_SIZE / read_time_seconds;
        printf("Throughput: %ld bytes/second\n", throughput);
        // displya the read data
        printf("Parent process read: %s\n", buffer);

        if (remove(FILE_PATH) == -1)
        {
            perror("remove");
            exit(EXIT_FAILURE);
        }
    }
}
void mmap_throughput()
{
    srand(time(NULL));

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int shared_memory_fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
    if (shared_memory_fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shared_memory_fd, SHARED_MEMORY_SIZE) == -1)
    {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    void *shared_memory = mmap(0, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, shared_memory_fd, 0);
    if (shared_memory == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    gettimeofday(&end, NULL);

    long connection_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    // convert to seconds
    double connection_time_seconds = connection_time / 1000000.0;
    printf("Latency before connecting to the shared memory: %.6f seconds\n", connection_time_seconds);

    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0)
    {
        printf("Child process is writing random letters to shared memory...\n");

        char data_to_write[DATA_SIZE];
        for (int i = 0; i < DATA_SIZE - 1; i++)
        {
            data_to_write[i] = generateRandomLetter();
        }
        data_to_write[DATA_SIZE - 1] = '\0';

        strncpy((char *)shared_memory, data_to_write, strlen(data_to_write) + 1);

        printf("Child process wrote: %s\n", data_to_write);
    }
    else
    {
        wait(NULL);

        gettimeofday(&start, NULL); // Start timing the read operation

        printf("Parent is reading from memory now...\n");

        char data_read[DATA_SIZE];
        strncpy(data_read, (char *)shared_memory, DATA_SIZE);

        gettimeofday(&end, NULL); // End timing the read operation

        long read_time = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
        double read_time_seconds = read_time / 1000000.0;

        printf("Parent process read: %s\n", data_read);
        printf("Time taken for reading: %.6f seconds\n", read_time_seconds);

        // Calculate throughput (bytes per second)
        long throughput = DATA_SIZE / read_time_seconds;
        printf("Throughput: %ld bytes/second\n", throughput);

        munmap(shared_memory, SHARED_MEMORY_SIZE);
        close(shared_memory_fd);

        shm_unlink("/my_shared_memory");
    }

    
}
