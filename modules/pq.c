#include "../include/pq.h"
#include "../include/flags.h"
#include <string.h>

#define parent(n) ((n - 1) / 2)    // Macros for parent and children of node
#define left_child(n) (2 * n + 1)
#define right_child(n) (2 * n + 2)

#define MIN_CAPACITY 64     // Default minimum capacity
#define EXP_FACTOR 2        // Default expansion factor

struct priority_queue
{
    void **heap;            // Resizable array
    size_t size;            // Number of elements in PQ
    size_t capacity;        // Number of elements current array can hold
    size_t min_capacity;    // Minimum capacity array can shrink to
    double exp_factor;      // Expansion factor, array capacity is multiplied by this number when it grows in size
    size_t prev_cap;        // New capacity array will shrink to
    cmpFunc cmp;
    destroyFunc destroy;
    int flag;
};

PQ pq_init(cmpFunc cmp, destroyFunc destroy)
{
    if (!cmp) return NULL;

    PQ pq = malloc(sizeof(struct priority_queue));
    if (!pq) return NULL;

    pq->heap = malloc(MIN_CAPACITY * sizeof(void *));

    if (!pq->heap)
    {
        free(pq); return NULL;
    }

    pq->size = 0;
    pq->capacity = pq->min_capacity = MIN_CAPACITY;
    pq->exp_factor = EXP_FACTOR;
    pq->prev_cap = pq->capacity / pq->exp_factor;

    pq->cmp = cmp;
    pq->destroy = destroy;
    pq->flag = OK;

    return pq;
}

PQ PQ_init_custom(size_t min_capacity, double exp_factor, cmpFunc cmp, destroyFunc destroy)
{
    PQ pq = malloc(sizeof(struct priority_queue));
    if (!pq) return NULL;

    pq->heap = malloc(min_capacity * sizeof(void *));

    if (!pq->heap)
    {
        free(pq); return NULL;
    }

    pq->size = 0;
    pq->capacity = pq->min_capacity = min_capacity;
    pq->exp_factor = exp_factor;
    pq->prev_cap = pq->capacity / pq->exp_factor;

    pq->cmp = cmp;
    pq->destroy = destroy;
    pq->flag = OK;

    return pq;
}

bool pq_empty(PQ pq)
{
    return (pq->size == 0);
}

size_t pq_size(PQ pq)
{
    return pq->size;
}

bool pq_insert(PQ pq, void *data)
{
    ERR_FUNC(pq, pq->cmp)

    // Double capacity if necessary
    if (pq->size == pq->capacity)
    {
        pq->capacity *= pq->exp_factor;

        void **new_heap = realloc(pq->heap, pq->capacity * sizeof(void *));
        ERR_ALLOC(pq, new_heap)

        pq->heap = new_heap;
    }

    // Start at the bottom of the heap
    size_t child = pq->size;
    size_t parent = parent(child);

    // While root hasn't been reached and elements have smaller priority than new element
    while (child && pq->cmp(data, pq->heap[parent]) < 0)
    {
        // Shift down elements until new element can be placed
        pq->heap[child] = pq->heap[parent];
        child = parent;
        parent = parent(child);
    }

    pq->heap[child] = data;
    pq->size++;

    return true;
}

void *pq_remove(PQ pq)
{ 
    ERR_EMPTY(pq)

    void *max_item = pq->heap[0];          // Element that is returned, found at the top of the heap
    void *cur_item = pq->heap[--pq->size]; // Element that should shift position, the last element in the heap

    size_t current = 0;                    // Position shifted element will be placed in, place at the top at first
    size_t left = left_child(current);     // Left and right children of that position
    size_t right = right_child(current);
    size_t new = 0;                        // Position current position may change to

    // While current node has at least a left child
    while (left < pq->size)
    {
        // If current node has at least one child with a higher priority,
        // set new position to the child that has the highest
        if (pq->cmp(pq->heap[left], cur_item) < 0)
            new = left;

        if (right < pq->size && pq->cmp(pq->heap[right], cur_item) < 0)
        {
            if (new == current)
                new = right;
            else
                new = pq->cmp(pq->heap[right], pq->heap[left]) < 0 ? right : left;
        }

        // If current position hasn't changed, place shifted element here
        if (new == current)
            break;

        // Change element of current position to element of new position
        pq->heap[current] = pq->heap[new];

        // Change current position to new position and find its children
        current = new;
        left = left_child(current);
        right = right_child(current);
    }

    // Place shifted element
    pq->heap[current] = cur_item;

    // Resize array if necessary
    if (pq->capacity >= 2 * pq->size * pq->exp_factor && pq->prev_cap >= pq->min_capacity)
    {
        pq->capacity = pq->prev_cap;
        pq->prev_cap = pq->capacity / pq->exp_factor;

        void **new_heap = realloc(pq->heap, pq->capacity * sizeof(void *));
        ERR_ALLOC(pq, new_heap)

        pq->heap = new_heap;
    }

    return max_item;
}

void *pq_peek(PQ pq)
{
    ERR_EMPTY(pq)
    return pq->heap[0];
}

bool pq_contains(PQ pq, void *data, cmpFunc cmp)
{
    ERR_FUNC(pq, cmp)

    for (size_t i = 0; i < pq->size; i++)
        if (!cmp(pq->heap[i], data))
            return true;

    return false;
}

PQ pq_copy(PQ pq, copyFunc copy)
{
    // Initialize new PQ
    PQ pq2 = malloc(sizeof(struct priority_queue));
    ERR_ALLOC(pq, pq2)

    // Allocate memory for new heap array
    pq2->heap = malloc(pq->capacity * sizeof(void *));
    
    if (!pq2->heap)
    {
        pq->flag = ALLOC;
        free(pq2); return NULL;
    }

    // If copy function is given, do a deep copy
    if (copy)
    {
        for (size_t i = 0; i < pq->size; i++)
        {
            pq2->heap[i] = copy(pq->heap[i]);

            // Failed to allocate memory
            if (!pq2->heap[i])
            {
                if (pq->destroy)
                {
                    for (size_t j = 0; j < i; j++)
                        pq->destroy(pq2->heap[j]);
                }
                
                free(pq2->heap); free(pq2);
                pq->flag = ALLOC; return NULL;
            }
        }
    }
    // Else do a shallow copy
    else memcpy(pq2->heap, pq->heap, pq->size * sizeof(void *));

    pq2->size = pq->size;
    pq2->capacity = pq->capacity;
    pq2->min_capacity = pq->min_capacity;
    pq2->exp_factor = pq->exp_factor;

    pq2->cmp = pq->cmp;
    pq2->destroy = pq->destroy;
    pq2->flag = OK;

    return pq2;
}

bool pq_clear(PQ pq)
{
    // Destroy all elements in PQ
    if (pq->destroy)
        for (size_t i = 0; i < pq->size; i++)
            pq->destroy(pq->heap[i]);

    free(pq->heap);

    // Allocate new heap at minimum capacity
    pq->heap = malloc(pq->min_capacity * sizeof(void *));
    ERR_ALLOC(pq, pq->heap)

    pq->size = 0;
    pq->capacity = pq->min_capacity;

    return true;
}

void pq_destroy(PQ pq)
{
    if (pq->destroy)
        for (size_t i = 0; i < pq->size; i++)
            pq->destroy(pq->heap[i]);

    free(pq->heap);
    free(pq);
}

size_t pq_min_capacity(PQ pq)
{
    return pq->min_capacity;
}

double pq_exp_factor(PQ pq)
{
    return pq->exp_factor;
}

void pq_set_destroy(PQ pq, destroyFunc destroy)
{
    pq->destroy = destroy;
}

bool pq_set_min_capacity(PQ pq, size_t min_capacity)
{
    if (!min_capacity)
    {
        pq->flag = ARG;
        return false;
    }

    pq->min_capacity = min_capacity;
    return true;
}

bool pq_set_exp_factor(PQ pq, double exp_factor)
{
    if (exp_factor <= 1)
    {
        pq->flag = ARG;
        return false;
    }

    pq->exp_factor = exp_factor;
    return true;
}

int pq_flag(PQ pq)
{
    return pq->flag;
}