#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include "func.h"


// A linear data structure where each element is connected with the element
// after and the element before it via pointers. This list supports the
// following basic operations, along with a few more:

// - get:      return element
// - append:   add an element at the end of the list
// - prepend:  add an element at the start of the list
// - insert:   place a new element before element at index 
// - remove:   destroy element
// - pop:      remove element without destroying it and return it

// Operations at the start or end of the list take constant time, while
// operations in the middle require traversal of at most half the list, and
// so take linear time.

typedef struct list *List;


// Initialize a list and return it, or return NULL in case of failure
// Set destroy to NULL so that elements in list are not destroyed when deletion functions are called
List list_init(destroyFunc destroy);

// Return true if list is empty
bool list_empty(List list);

// Return number of elements in list
size_t list_size(List list);

// Return element that is equal to data in list or NULL if element is not found (requires cmp func)
void *list_get(List list, void *data, cmpFunc cmp);

// Return first element of list, or NULL in case of failure
void *list_get_first(List list);

// Return last element of list, or NULL in case of failure
void *list_get_last(List list);

// Return element at index of list, or NULL in case of failure
void *list_get_at(List list, size_t index);

// Insert element at the start of the list, return true if successful
bool list_prepend(List list, void *data);

// Insert element at the end of the list, return true if successful
bool list_append(List list, void *data);

// Insert element before element at index of list, return true if successful
// You cannot insert in an empty list or after the last element. Use append for those insertions.
bool list_insert(List list, void *data, size_t index);

// Remove and destroy element that is equal to data, return true if successful (requires cmp func)
// Data must exist in list or structure is flagged with ERR_ARG
bool list_remove(List list, void *data, cmpFunc cmp);

// Remove and destroy all elements that are equal to data, return true if successful (requires cmp func)
bool list_remove_all(List list, void *data, cmpFunc cmp);

// Remove and destroy first element of list, return true if successful
bool list_remove_first(List list);

// Remove and destroy last element of list, return true if successful
bool list_remove_last(List list);

// Remove and destroy element at index of list, return true if successful
bool list_remove_at(List list, size_t index);

// Remove and return element that is equal to data, NULL in case of failure (requires cmp func)
// Data must exist in list or structure is flagged with ERR_ARG
void *list_pop(List list, void *data, cmpFunc cmp);

// Remove and return first element, NULL in case of failure
void *list_pop_first(List list);

// Remove and return last element, NULL in case of failure
void *list_pop_last(List list);

// Remove and return element at index of list, NULL in case of failure
void *list_pop_at(List list, size_t index);

// Return number of times data appears in list (requires cmp func)
size_t list_count(List list, void *data, cmpFunc cmp);

// Return true if list contains data (requires cmp func)
bool list_contains(List list, void *data, cmpFunc cmp);

// Reverse order of elements in list
void list_reverse(List list);

// Remove elements before start index and from end index onwards, return true if successful
bool list_trim(List list, size_t start, size_t end);

// Sort list, return true if successful (requires cmp func)
bool list_sort(List list, cmpFunc cmp);

// Insert data in sorted manner, return true if successful (requires cmp func)
// If initial list is unsorted, behavior is undefined
bool list_insert_sorted(List list, void *data, cmpFunc cmp);

// Return a deep copy of list if copy func is given or a shallow copy if copy func is NULL
// Return NULL in case of failure
List list_copy(List list, copyFunc copy);

// Remove all elements from list
void list_clear(List list);

// Free memory allocated for list
void list_destroy(List list);

// Change destroy function for list
void list_set_destroy(List list, destroyFunc destroy);

// Return list flag
int list_flag(List list);