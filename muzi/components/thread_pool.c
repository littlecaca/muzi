#include <pthread.h>

#include "atomic.h"

typedef void *(*task_func)(void *);

typedef struct _task
{
    void *next;
    task_func func;
    void *args;
} Task;

typedef struct _task_queue
{
    void *head;
    void **tail;
    int block;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} TaskQueue;

typedef struct _thread_pool
{
    TaskQueue *queue;
    

} ThreadPool;
