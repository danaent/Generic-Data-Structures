#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include "func.h"


// A linear data structure that follows the LIFO (last in, first out) principle.
// It has three basic operations:

// - push: add an element to the top of the stack
// - pop:  remove and return element at the top of the stack
// - peek: return element at the top without removing it

// The stack is implemented with a singly linked list and its basic operations
// are performed in constant time.

typedef struct stack *Stack;



// Initialize a stack and return it, or return NULL in case of failure
// Set destroy to NULL so that elements in stack are not destroyed when deletion functions are called
Stack stack_init(destroyFunc destroy);

// Return true if stack is empty
bool stack_empty(Stack stack);

// Return stack size
size_t stack_size(Stack stack);

// Push an element to the top of the stack, return true if successful
bool stack_push(Stack stack, void *data);

// Return stack's top element without removing it, stack cannot be empty
void *stack_peek(Stack stack);

// Remove top element from stack and return it, stack cannot be empty
void *stack_pop(Stack stack);

// Return true if stack contains element (requires cmp func)
bool stack_contains(Stack stack, void *data, cmpFunc cmp);

// Return a deep copy of stack if copy func is given or a shallow copy if copy func is NULL
// Return NULL in case of failure
Stack stack_copy(Stack stack, copyFunc copy);

// Remove all elements from stack
void stack_clear(Stack stack);

// Free memory allocated for stack
void stack_destroy(Stack stack);

// Change destroy function for stack
void stack_set_destroy(Stack stack, destroyFunc destroy);

// Return stack flag
int stack_flag(Stack stack);