#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define THREAD_COUNT 4
#define TASK_COUNT 10

// Structure to hold a task
typedef struct {
    int id;
} Task;

// Structure to hold the thread pool
typedef struct {
    pthread_t threads[THREAD_COUNT];
    Task *tasks;
    int task_count;
    int task_index;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool stop;
} ThreadPool;

// Function executed by worker threads
void *worker_thread(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;
    while (true) {
        pthread_mutex_lock(&pool->mutex);
        // Wait for a task or a stop signal
        while (pool->task_index >= pool->task_count && !pool->stop) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        // Exit if stop signal received and no tasks
        if (pool->stop && pool->task_index >= pool->task_count) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        // Get a task
        int task_id = pool->tasks[pool->task_index].id;
        pool->task_index++;
        pthread_mutex_unlock(&pool->mutex);
        // Execute the task
        printf("Thread %lu executing task %d\n", pthread_self(), task_id);
        sleep(1); // Simulate task execution
    }
    printf("Thread %lu exiting\n", pthread_self());
    return NULL;
}

// Function to initialize the thread pool
ThreadPool *thread_pool_init() {
    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (pool == NULL) {
        perror("Failed to allocate thread pool");
        exit(EXIT_FAILURE);
    }
    pool->task_count = 0;
    pool->task_index = 0;
    pool->stop = false;
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }
    return pool;
}

// Function to add a task to the thread pool
void thread_pool_add_task(ThreadPool *pool, int task_id) {
    pthread_mutex_lock(&pool->mutex);
    pool->task_count++;
    pool->tasks = (Task *)realloc(pool->tasks, pool->task_count * sizeof(Task));
    if (pool->tasks == NULL) {
        perror("Failed to reallocate tasks");
        exit(EXIT_FAILURE);
    }
    pool->tasks[pool->task_count - 1].id = task_id;
    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
}

// Function to destroy the thread pool
void thread_pool_destroy(ThreadPool *pool) {
    pthread_mutex_lock(&pool->mutex);
    pool->stop = true;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    free(pool->tasks);
    free(pool);
}

int main() {
    ThreadPool *pool = thread_pool_init();
    // Add tasks to the thread pool
    for (int i = 0; i < TASK_COUNT; i++) {
        thread_pool_add_task(pool, i);
    }
    // Wait for all tasks to complete
    while (pool->task_index < pool->task_count);
    // Destroy the thread pool
    thread_pool_destroy(pool);
    printf("Thread pool destroyed.\n");
    return 0;
}
