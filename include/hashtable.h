#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include "func.h"


// An unordered map where each key is mapped to a value. Keys are used to
// index the values into the table via a hash function. The basic operations
// of a hash table are:

// - insert: insert a key-value pair in the table
// - search: return the value associated with the given key in the table
// - remove: remove the value associated with the given key from the table

// The hash table is implemented with separate chaining and rehashing for
// collision handling. This means that when two elements are hashed to the same
// index, they are inserted into a linked list. Since this could result in long
// searching times when the linked list grows, the table is resized once the
// ratio of elements to table size grows beyond a set number - the load factor -
// and the elements are rehashed and inserted back into the table. This allows
// for constant time on average and linear time in the worst case for all
// operations. However, this performance also greatly depends on the load factor
// and the hash function being used. The load factor is set to 0.75 by default
// but can be changed. For a faster performance that uses more space, you can
// lower the load factor, but if space is an issue a higher load factor is better.

// Since this hash table is implemented as a map, keys are unique. If a key is
// inserted into the table when it's already in it, the previous value associated
// with it is destroyed and replaced by the new one.

typedef struct hashtable *Hashtable;


// Initialize a hash table and return it, or return NULL in case of failure
// Set destroy to NULL so that elements in table are not destroyed when deletion functions are called
// Hash and cmp functions are required
Hashtable hash_init(hashFunc key_hash, cmpFunc key_cmp, destroyFunc key_destroy, destroyFunc val_destroy);

// Return true if hash table is empty
bool hash_empty(Hashtable table);

// Return number of elements in hash table
size_t hash_size(Hashtable table);

// Insert key-value pair into hash table, return true if successful
bool hash_insert(Hashtable table, void *key, void *value);

// Return value associated with key in the hash table, or NULL if key is not in the table
void *hash_search(Hashtable table, void *key);

// Remove key and value associated with it from hash table, return false if key was not in table
bool hash_remove(Hashtable table, void *key);

// Remove all key-value pairs from hash table, return true if successful
bool hash_clear(Hashtable table);

// Free memory allocated for table
void hash_destroy(Hashtable table);

// Return hash table load factor
double hash_load_factor(Hashtable table);

// Change key-destroy function for hash table
void hash_set_key_destroy(Hashtable table, destroyFunc key_destroy);

// Change value-destroy function for hash table
void hash_set_val_destroy(Hashtable table, destroyFunc val_destroy);

// Change load factor for hash table and resize according to new load factor, return true if successful
bool hash_set_load_factor(Hashtable table, double load_factor);

// Return hash table flag
int hash_flag(Hashtable table);