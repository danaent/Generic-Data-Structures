#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include "func.h"


// A self-balancing binary search tree that functions as an ordered set. Each
// node is colored red or black in order to satisfy the following properties:

// - The root is black
// - Leaf nodes (NULL nodes) are black
// - The children of a red node are black
// - Every path from a node to one of its descendants has the same number of
//   black nodes

// Because the tree remains relatively balanced, insertion, removal and
// searching are guaranteed to take logarithmic time. Duplicates are not
// allowed in the tree.

typedef struct rbtree *RBTree;


// Initialize a Red-Black Tree and return it, or return NULL in case of failure
// Set destroy to NULL so that elements in tree are not destroyed when deletion functions are called
// Cmp function is required
RBTree rbt_init(cmpFunc cmp, destroyFunc destroy);

// Return true if tree is empty
bool rbt_empty(RBTree rbt);

// Return number of elements in tree
size_t rbt_size(RBTree rbt);

// Insert data into the tree, return true if successful
bool rbt_insert(RBTree rbt, void *data);

// Return element in tree that is equal to data, or NULL if such element does not exist
void *rbt_search(RBTree rbt, void *data);

// Remove data from tree, return false if data was not in tree
bool rbt_remove(RBTree rbt, void *data);

// Return element with minimum priority in tree, or NULL in case of failure
void *rbt_min(RBTree rbt);

// Return element with maximum priority in tree, or NULL in case of failure
void *rbt_max(RBTree rbt);

// Return a deep copy of tree if copy func is given or a shallow copy if copy func is NULL
// Return NULL in case of failure
void *rbt_copy(RBTree rbt, copyFunc copy);

// Remove all elements from tree
void rbt_clear(RBTree rbt);

// Free memory allocated for tree
void rbt_destroy(RBTree rbt);

// Change destroy function for tree
void rbt_set_destroy(RBTree rbt, destroyFunc destroy);

// Return tree flag
int rbt_flag(RBTree rbt);