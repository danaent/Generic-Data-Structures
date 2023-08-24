#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include "func.h"


// A balanced multi-way search tree that functions as an ordered set. A B-tree
// must satisfy the following properties:

// - Each internal node except the root has at most B and at least ⌈B/2⌉ children
// - All external nodes are on the same level.

// Since the tree remains balanced, insertion, removal and searching are guaranteed
// to take logarithmic time. Given that it's a set, duplicates are not allowed.

typedef struct btree *BTree;


// Initialize a B-tree with b >= 3 and return it, or return NULL in case of failure
// Set destroy to NULL so that elements in tree are not destroyed when deletion functions are called
// Cmp function is required
BTree btree_init(int B, cmpFunc cmp, destroyFunc destroy);

// Return true if tree is empty
bool btree_empty(BTree abtree);

// Return number of elements in tree
size_t btree_size(BTree abtree);

// Insert data into the tree, return true if successful
bool btree_insert(BTree abtree, void *data);

// Return element in tree that is equal to data, or NULL if such element does not exist
void *btree_search(BTree abtree, void *data);

// Remove data from tree, return false if data was not in tree
bool btree_remove(BTree abtree, void *data);

// Return element with minimum priority in tree, or NULL in case of failure
void *btree_min(BTree abtree);

// Return element with maximum priority in tree, or NULL in case of failure
void *btree_max(BTree abtree);

// Return a deep copy of tree if copy func is given or a shallow copy if copy func is NULL
// Return NULL in case of failure
BTree btree_copy(BTree abtree, copyFunc copy);

// Remove all elements from tree
void btree_clear(BTree abtree);

// Free memory allocated for tree
void btree_destroy(BTree abtree);

// Return tree flag
int btree_flag(BTree abtree);