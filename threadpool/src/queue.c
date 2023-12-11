#include "queue.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

queue_t *queue_init(void) {
    queue_t *new_queue = malloc(sizeof(queue_t));
    new_queue->head = malloc(sizeof(node_t));
    new_queue->tail = malloc(sizeof(node_t));

    new_queue->head->next = new_queue->tail;
    new_queue->tail->prev = new_queue->head;

    pthread_mutex_init(&new_queue->mutex, NULL);
    pthread_cond_init(&new_queue->cond, NULL);
    new_queue->size = 0;
    return new_queue;
}

void queue_enqueue(queue_t *queue, void *value) {
    pthread_mutex_lock(&queue->mutex);
    node_t *new_node = malloc(sizeof(node_t));
    new_node->value = value;

    queue->tail->prev->next = new_node;
    queue->tail->prev = new_node;
    new_node->prev = queue->tail->prev;
    new_node->next = queue->tail;
    queue->size = queue->size + 1;
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

void *queue_dequeue(queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);
    void *value = NULL;
    while (queue->size == 0) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    value = queue->head->next->value;
    node_t *node_to_dequeue = queue->head->next;
    node_to_dequeue->next->prev = queue->head;
    queue->head->next = queue->head->next->next;
    free(node_to_dequeue);
    queue->size = queue->size - 1;
    pthread_mutex_unlock(&queue->mutex);
    return value;
}

void queue_free(queue_t *queue) {
    free(queue->head);
    free(queue->tail);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    free(queue);
}