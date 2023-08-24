#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include "func.h"


// A type of queue where each element is associated with a priority value.
// Elements with higher priority are dequeued before elements with lower
// priority. The order of elements with the same priority is undefined. A PQ
// has three basic operations:

// - insert: insert an element into the PQ
// - remove: remove and return element with highest priority
// - peek:   return element with highest priority without removing it

// Here the PQ is implemented using a binary heap, which is a complete tree
// where each node has a greater than or equal priority to its children. This
// allows for logarithmic time for insertion and removal and constant time for
// peeking. The binary heap is represented with a dynamic array that resizes
// similarly to the vector. Min capacity and expansion factor are set to 64
// and 2 by default.

typedef struct priority_queue* PQ;



// Initialize a PQ and return it, or return NULL in case of failure
// Set destroy to NULL so that elements in PQ are not destroyed when deletion functions are called
// Cmp function is required
PQ pq_init(cmpFunc cmp, destroyFunc destroy);

// Return true if PQ is empty
bool pq_empty(PQ pq);

// Return PQ size
size_t pq_size(PQ pq);

// Insert an element into PQ, return true if successful
bool pq_insert(PQ pq, void *new_data);

// Remove and return element with highest priority from PQ, or return NULL in case of failure
void *pq_remove(PQ pq);

// Return element with highest priority in PQ without removing it, or NULL in case of failure
void *pq_peek(PQ pq);

// Return true if PQ contains element (requires cmp func)
bool pq_contains(PQ pq, void *data, cmpFunc cmp);

// Return a deep copy of PQ if copy func is given or a shallow copy if copy func is NULL
// Return NULL in case of failure
PQ pq_copy(PQ pq, copyFunc copy);

// Remove all elements from PQ, return true if successful
bool pq_clear(PQ pq);

// Free memory allocated for PQ
void pq_destroy(PQ pq);

// Return heap's minimum capacity - heap cannot become smaller than this
size_t pq_min_capacity(PQ pq);

// Return heap expansion factor - multiply capacity by exp factor when element can't fit
double pq_exp_factor(PQ pq);

// Change destroy function for PQ
void pq_set_destroy(PQ pq, destroyFunc destroy);

// Change PQ min capacity to >0, return true if successful
bool pq_set_min_capacity(PQ pq, size_t min_capacity);

// Change PQ expansion factor to >1, return true if successful
bool pq_set_exp_factor(PQ pq, double exp_factor);

// Return PQ flag
int pq_flag(PQ pq);