#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include "func.h"


// A linear data structure that is open on both ends and follows the FIFO
// (first in, first out) principle. It has three basic operations:

// - enqueue: add an element at the end of the queue
// - dequeue: remove and return the element at the front of the queue
// - peek:    return element at the front without removing it

// The queue is implemented with a singly linked list and its basic operations
// are performed in constant time.

typedef struct queue *Queue;



// Initialize a queue and return it, or return NULL in case of failure
// Set destroy to NULL so that elements in stack are not destroyed when deletion functions are called
Queue queue_init(destroyFunc destroy);

// Return true if queue is empty
bool queue_empty(Queue queue);

// Return queue size
size_t queue_size(Queue queue);

// Add an element at the end of the queue, return true if successful
bool queue_enqueue(Queue queue, void *data);

// Return queue's front element without removing it, queue cannot be empty
void *queue_peek(Queue queue);

// Remove element from the front of the queue and return it, queue cannot be empty
void *queue_dequeue(Queue queue);

// Return true if queue contains element (requires cmp func)
bool queue_contains(Queue queue, void *data, cmpFunc cmp);

// Return a deep copy of queue if copy func is given or a shallow copy if copy func is NULL
// Return NULL in case of failure
Queue queue_copy(Queue queue, copyFunc copy);

// Remove all elements from queue
void queue_clear(Queue queue);

// Free memory allocated for queue
void queue_destroy(Queue queue);

// Change destroy function for queue
void queue_set_destroy(Queue queue, destroyFunc destroy);

// Return queue flag
int queue_flag(Queue queue);