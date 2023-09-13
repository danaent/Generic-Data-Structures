// FUNCTIONS USED BY THE DATA STRUCTURES

// Free all memory allocated for element. When NULL is passed to the 
// function, it should have no effect.
// This function is required to initialize all data structures and is used by
// functions that delete elements. When set to NULL, memory is not de-allocated
// for the elements held by the pointers stored in the data structure.

typedef void (*destroyFunc)(void *);

// Return an integer that determines the order of the two elements. If p1 and
// p2 are passed into the function, its return values should be:

// < 0 if p1 precedes p2, or has a higher priority
// = 0 if p1 has the same priority as p2
// > 0 if p1 follows p2, or has a lower priority

// This function is required for data structures where each element is
// associated with a priority, such as the priority queue or the Red-Black Tree.
// It cannot be NULL.

typedef int (*cmpFunc)(const void *, const void *);

// Return a copy of element. If the function returns NULL, memory allocation
// failure is assumed. 
// This function is required in functions that return a copy of a data
// structure. If set to NULL, the pointers of the structure are copied, but
// not the elements they hold.

typedef void *(*copyFunc)(const void *);

// Hash element to an unsigned integer.
// This function is required for the hash table. It cannot be NULL.

typedef size_t (*hashFunc)(const void *);