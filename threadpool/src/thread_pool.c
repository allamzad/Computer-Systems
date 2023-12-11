#include "thread_pool.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct thread_pool {
    pthread_t *workers;
    queue_t *work;
    size_t num_workers;
} thread_pool_t;

typedef struct funct {
    work_function_t work_function;
    void *aux;
} funct_t;

void thread_execute(void *p) {
    thread_pool_t *pool = (thread_pool_t *) p;
    assert(pool != NULL);

    void *work = queue_dequeue(pool->work);
    while (work != NULL) {
        funct_t *funct = (funct_t *) work;
        work_function_t function = (work_function_t) funct->work_function;
        void *aux = funct->aux;
        function(aux);
        free(funct);
        work = queue_dequeue(pool->work);
    }
    return;
}

thread_pool_t *thread_pool_init(size_t num_worker_threads) {
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    pool->workers = malloc(sizeof(pthread_t) * num_worker_threads);
    pool->num_workers = num_worker_threads;
    pool->work = queue_init();

    // Starting the threads
    for (size_t i = 0; i < num_worker_threads; i++) {
        pthread_create(&(pool->workers[i]), NULL, (void *) thread_execute, (void *) pool);
    }

    return pool;
}

void thread_pool_add_work(thread_pool_t *pool, work_function_t function, void *aux) {
    // Adding tasks to the work queue
    funct_t *f = malloc(sizeof(funct_t));
    f->work_function = function;
    f->aux = aux;
    queue_enqueue(pool->work, (void *) f);
}

void thread_pool_free(thread_pool_t *pool) {
    queue_free(pool->work);
    free(pool->workers);
    free(pool);
}

void thread_pool_finish(thread_pool_t *pool) {
    // Adding NULLs to the back of the work queue
    for (size_t i = 0; i < pool->num_workers; i++) {
        queue_enqueue(pool->work, NULL);
    }
    for (size_t i = 0; i < pool->num_workers; i++) {
        pthread_join(pool->workers[i], NULL);
    }
    thread_pool_free(pool);
}
