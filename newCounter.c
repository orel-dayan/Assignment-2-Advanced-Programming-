#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>


#define MAX_QUEUE_SIZE 10000
#define BATCH_SIZE 10000

typedef struct
{
    int data[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    bool isProcessingComplete;
} PrimeQueue;

typedef struct
{
    PrimeQueue *queue;
    int *totalPrimeCount;
    pthread_mutex_t *countMutex;
} WorkerThreadArgs;

/***
 * Function declarations

 */
PrimeQueue *createPrimeQueue();
void addNumberToQueue(PrimeQueue *q, int data);
int removeNumbersFromQueue(PrimeQueue *queue, int batch[], int *batchSize);
void *processPrimes(void *args);
bool isPrime(int number);

int main() {
    int inputNumber;
    int totalPrimeCount = 0;
    int threadCount = 4;

    pthread_t *threads = malloc(threadCount * sizeof(pthread_t));
    WorkerThreadArgs *workerArgs = malloc(threadCount * sizeof(WorkerThreadArgs));
    PrimeQueue *primeQueue = createPrimeQueue();
    pthread_mutex_t countMutex;

    pthread_mutex_init(&countMutex, NULL);

    for (int i = 0; i < threadCount; i++) {
        workerArgs[i].queue = primeQueue;
        workerArgs[i].totalPrimeCount = &totalPrimeCount;
        workerArgs[i].countMutex = &countMutex;
        pthread_create(&threads[i], NULL, processPrimes, &workerArgs[i]);
    }

    while (scanf("%d", &inputNumber) != EOF) {
        addNumberToQueue(primeQueue, inputNumber);
    }

    pthread_mutex_lock(&primeQueue->mutex);
    primeQueue->isProcessingComplete = true;
    pthread_cond_broadcast(&primeQueue->condition);
    pthread_mutex_unlock(&primeQueue->mutex);

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("%d total primes.\n", totalPrimeCount);

    pthread_mutex_destroy(&countMutex);
    pthread_mutex_destroy(&primeQueue->mutex);
    pthread_cond_destroy(&primeQueue->condition);

    free(primeQueue);
    free(threads);
    free(workerArgs);

    return 0;
}

/**
 * Helper functions
 
 */
PrimeQueue *createPrimeQueue()
{
    PrimeQueue *queue = (PrimeQueue *)malloc(sizeof(PrimeQueue));
    queue->front = queue->rear = queue->count = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->condition, NULL);
    queue->isProcessingComplete = false;
    return queue;
}

void addNumberToQueue(PrimeQueue *queue, int number)
{
    pthread_mutex_lock(&queue->mutex);
    while (queue->count == MAX_QUEUE_SIZE)
    {
        pthread_cond_wait(&queue->condition, &queue->mutex);
    }
    queue->data[queue->rear] = number;
    queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
    queue->count++;
    pthread_cond_signal(&queue->condition);
    pthread_mutex_unlock(&queue->mutex);
}

int removeNumbersFromQueue(PrimeQueue *queue, int batch[], int *batchSize)
{
    pthread_mutex_lock(&queue->mutex);
    while (queue->count == 0 && !queue->isProcessingComplete)
    {
        pthread_cond_wait(&queue->condition, &queue->mutex);
    }
    if (queue->count == 0)
    {
        pthread_mutex_unlock(&queue->mutex);
        return -1; // Queue is empty and processing is complete
    }
    *batchSize = 0;
    while (*batchSize < BATCH_SIZE && queue->count > 0)
    {
        batch[*batchSize] = queue->data[queue->front];
        queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
        queue->count--;
        (*batchSize)++;
    }
    pthread_cond_signal(&queue->condition);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}
/**
 * Function to check if a number is prime 
 * Improved by reducing the number of iterations
 


 */
bool isPrime(int number)
{
    if (number <= 1) // 0 and 1 are not prime numbers       
        return false;
    if (number <= 3) // 2 and 3 are prime numbers
        return true;
    if (number % 2 == 0 || number % 3 == 0) // Eliminate multiples of 2 and 3
        return false;
    for (int i = 5; i * i <= number; i += 6)
    {
        if (number % i == 0 || number % (i + 2) == 0) // check for multiples of 6
            return false;
    }
    return true;
}

void *processPrimes(void *args)
{
    WorkerThreadArgs *threadArgs = (WorkerThreadArgs *)args;
    PrimeQueue *queue = threadArgs->queue;
    int localPrimeCount = 0;
    while (true)
    {
        int batch[BATCH_SIZE];
        int batchSize;
        if (removeNumbersFromQueue(queue, batch, &batchSize) == -1)
        {
            break;
        }
        for (int i = 0; i < batchSize; i++)
        {
            if (isPrime(batch[i]))
            {
                localPrimeCount++;
            }
        }
    }
    pthread_mutex_lock(threadArgs->countMutex);
    *(threadArgs->totalPrimeCount) += localPrimeCount;
    pthread_mutex_unlock(threadArgs->countMutex);
    return NULL;
}