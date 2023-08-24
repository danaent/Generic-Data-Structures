#pragma once
#include <stdlib.h> 
#include <stdbool.h>
#include "func.h"


// A dynamic array that grows when its capacity is exceeded and shrinks when
// its elements are far fewer than its capacity. The array begins at a minimum
// capacity and never shrinks below that. When its capacity is reached, it is
// multiplied by a set number, its expansion factor. For this vector min
// capacity and expansion factor are initialized to 64 and 2 but can be changed.

// This vector supports the following basic operations, along with a few more:

// - get:      return element in position
// - set:      destroy and replace element in position
// - append:   add an element at the end of the vector
// - prepend:  add an element at the start of the vector by shifting every
//             element to the right
// - insert:   place new element in position by shifting every element from that
//             position to the right
// - remove:   destroy element in position and shift elements to the left to fill
//             empty cell 
// - pop:      remove element without destroying it, shift elements to the left
//             to fill empty cell and return removed element

// Get, set and append operations are performed in constant time, while prepend,
// insert, remove and pop in linear time.

// RESIZING AND ALLOCATION: Inserting and deleting elements may resize the
// array. If memory can't be allocated during insertion, the new element is not
// inserted and the vector remains the same. If memory can't be allocated during
// deletion, the element is deleted but the array doesn't shrink. See if
// insertion and deletion functions return false and check the structure's flag
// to know if this error has occured.

typedef struct vector *Vector;



// Initialize a vector and return it, or return NULL in case of failure
// Set destroy to NULL so that elements in vector are not destroyed when deletion functions are called
Vector vector_init(destroyFunc destroy);

// Initialize a vector with custom values and return it, or return NULL in case of failure
// Must be min_capacity > 0 and exp_factor > 1
// Set destroy to NULL so that elements in vector are not destroyed when deletion functions are called
Vector vector_init_custom(size_t min_capacity, double exp_factor, destroyFunc destroy);

// Return true if vector is empty
bool vector_empty(Vector vector);

// Return number of elements in vector
size_t vector_size(Vector vector);

// Remove first element and replace it with data, return true if successful
bool vector_set_first(Vector vector, void *data);

// Remove last element and replace it with data, return true if successful
bool vector_set_last(Vector vector, void *data);

// Remove element at index and replace it with data, return true if successful
bool vector_set_at(Vector vector, void *data, size_t index);

// Return first element of vector, or NULL in case of failure
void *vector_get_first(Vector vector);

// Return last element of vector, or NULL in case of failure
void *vector_get_last(Vector vector);

// Return element at index of vector, or NULL in case of failure
void *vector_get_at(Vector vector, size_t index);

// Insert element before first element of vector, return true if successful
bool vector_prepend(Vector vector, void *data);

// Insert element after last element of vector, return true if successful
bool vector_append(Vector vector, void *data);

// Insert element before element at index of vector, return true if successful
// You cannot insert in an empty vector or after the last element. Use append for those insertions.
bool vector_insert(Vector vector, void *data, size_t index);

// Remove and destroy first element of vector, return true if successful
bool vector_remove_first(Vector vector);

// Remove and destroy last element of vector, return true if successful
bool vector_remove_last(Vector vector);

// Remove and destroy element at index of vector, return true if successful
bool vector_remove_at(Vector vector, size_t index);

// Remove and destroy all instances of data in vector, return true if successful (requires cmp func)
bool vector_remove_all(Vector vector, void *data, cmpFunc cmp);

// Remove and return first element of vector, NULL in case of failure
void *vector_pop_first(Vector vector);

// Remove and return last element of vector, NULL in case of failure
void *vector_pop_last(Vector vector);

// Remove and return element at index of vector, NULL in case of failure
void *vector_pop_at(Vector vector, size_t index);

// Return index of first instance of data in vector, __SIZE_MAX__ in case of failure (requires cmp func)
// Data must exist in vector or structure is flagged with ERR_ARG
size_t vector_index(Vector vector, void *data, cmpFunc cmp);

// Return number of times data appears in vector (requires cmp func)
size_t vector_count(Vector vector, void *data, cmpFunc cmp);

// Return true if vector contains data (requires cmp func)
bool vector_contains(Vector vector, void *data, cmpFunc cmp);

// Return an array that contains elements from index at start to index at end-1, where end <= size
// If copy func is NULL, new array contains a shallow copy of the elements
void **vector_array(Vector vector, size_t start, size_t end, copyFunc copy);

// Reverse order of elements in vector
void vector_reverse(Vector vector);

// Remove elements before start and from end onwards, vector capacity changes to its new size
// If new capacity is smaller than min capacity, min capacity is changed
// This function can be used to change array's capacity to its size without removing any elements
bool vector_trim(Vector vector, size_t start, size_t end);

// Sort vector, return true if successful (requires cmp func)
bool vector_sort(Vector vector, cmpFunc cmp);

// Return index of data in vector, __SIZE_MAX__ in case of failure (requires cmp func)
// If vector is unsorted, behavior is undefined
// Vector must contain data exactly once or any index containing data could be returned
// If it doesn't contain data, structure is flagged with ERR_ARG
size_t vector_binary_search(Vector vector, void *data, cmpFunc cmp);

// Insert data in sorted manner, return true if successful
// If initial vector is unsorted, behavior is undefined
// Every insertion takes logarithmic time, so this is much slower than appending every element and then sorting
bool vector_insert_sorted(Vector vector, void *data, cmpFunc cmp);

// Return a deep copy of vector if copy func is given or a shallow copy if copy func is NULL
// Return NULL in case of failure
Vector vector_copy(Vector vector, copyFunc copy);

// Remove all elements from vector, return true if successful
bool vector_clear(Vector vector);

// Free memory allocated for vector
void vector_destroy(Vector vector);

// Return number of elements vector can hold
size_t vector_capacity(Vector vector);

// Return vector's minimum capacity - vector cannot become smaller than this
size_t vector_min_capacity(Vector vector);

// Return vector expansion factor - multiply capacity by exp factor when element can't fit
double vector_exp_factor(Vector vector);

// Change destroy function for vector
void vector_set_destroy(Vector vector, destroyFunc destroy);

// Change vector min capacity to >0, return true if successful
// If min capacity is bigger than current capacity, capacity changes to new min
bool vector_set_min_capacity(Vector vector, size_t min_capacity);

// Change vector expansion factor, return true if successful
bool vector_set_exp_factor(Vector vector, double exp_factor);

// Return vector flag
int vector_flag(Vector vector);