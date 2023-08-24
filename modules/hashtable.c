#include "../include/hashtable.h"
#include "../include/flags.h"

#define MAX_PRIME 2200178621
#define LOAD_FACTOR 0.75

typedef struct node *Node;

typedef struct node
{
    void *key;            // Key stored in node
    void *value;          // Value associated with the key
    size_t hash;          // Hash value returned by the hash function
    Node next;            // Pointer to next node in list
}
node;

struct hashtable
{
    Node *buckets;         // Array of linked lists (buckets)
    size_t size;           // Number of elements in table
    size_t capacity;       // Number of elements table can hold
    size_t threshold;      // Maximum elements table can hold before resizing
    double load_factor;
    size_t cap_index;      // Index of capacity in global array
    hashFunc key_hash;
    cmpFunc key_cmp;
    destroyFunc key_destroy;
    destroyFunc val_destroy;
    int flag;
};


// Table sizes are prime numbers for more efficient modulo operations
size_t hash_sizes[] = {67, 131, 263, 523, 1049, 2099, 4201, 8389, 16787, 33577, 67153, 134293, 268573, 537143, 1074287, 2148599, 
4297201, 8594431, 17188861, 34377751, 68755571, 137511193, 275022313, 550044679, 1100089357, 2200178621};



////////////////////////////////////// FUNCTIONS FOR BUCKETS //////////////////////////////////////

// Free memory allocated for node
void node_destroy(Node node, destroyFunc key_destroy, destroyFunc val_destroy)
{
    if (key_destroy) key_destroy(node->key);
    if (val_destroy) val_destroy(node->value);
    free(node);
}

// Insert node at the start of the bucket
Node bucket_insert_node(Node bucket, Node node)
{
    node->next = bucket;
    return node;
}

// Insert a key-value pair at the start of the bucket
Node bucket_insert(Node bucket, void *key, void *value, size_t hash)
{
    Node node = malloc(sizeof(struct node));
    if (!node) return NULL;

    node->key = key;
    node->value = value;
    node->hash = hash;

    return bucket_insert_node(bucket, node);
}

// Return node of bucket that holds key, or NULL if such node does not exist
Node bucket_search(Node bucket, void *key, cmpFunc key_cmp)
{
    while (bucket && key_cmp(key, bucket->key))
        bucket = bucket->next;

    return bucket;
}

// Return true if bucket already contains key and replace both key and value
bool bucket_replace(Node bucket, void *key, void *value, cmpFunc key_cmp, destroyFunc key_destroy, destroyFunc val_destroy)
{
    // Search bucket for key
    bucket = bucket_search(bucket, key, key_cmp);

    // If bucket contains key, replace key and value
    if (bucket)
    {
        if (key_destroy) key_destroy(bucket->key);
        if (val_destroy) val_destroy(bucket->value);

        bucket->key = key;
        bucket->value = value;
        return true;
    }
    
    return false;
}

// Remove key and value from the bucket, set found to true if bucket contains key
Node bucket_remove(Node bucket, void *key, cmpFunc key_cmp, destroyFunc key_destroy, destroyFunc val_destroy, bool *found)
{
    Node cur_node = bucket;
    *found = false;

    // If bucket is empty, it does not contain key
    if (!cur_node) return NULL;

    // If key is in the head node
    if (!key_cmp(key, cur_node->key))
    {
        // Save next node and destroy current head
        Node temp_node = cur_node->next;
        node_destroy(cur_node, key_destroy, val_destroy);
        *found = true;

        // Return new head
        return temp_node;
    }

    // Otherwise traverse bucket nodes
    Node prev_node = cur_node;
    cur_node = cur_node->next;

    while (cur_node && key_cmp(key, cur_node->key))
    {
        prev_node = cur_node;
        cur_node = cur_node->next;
    }

    // Destroy node if found
    if (cur_node) 
    {
        prev_node->next = cur_node->next;
        node_destroy(cur_node, key_destroy, val_destroy);
        *found = true;
    }

    return bucket;
}

// Free memory for every node in bucket
void bucket_destroy(Node bucket, destroyFunc key_destroy, destroyFunc val_destroy)
{
    if (!bucket) return;

    Node next_node = bucket->next;

    while (bucket && bucket->next)
    {
        node_destroy(bucket, key_destroy, val_destroy);
        bucket = next_node;
        next_node = bucket->next;
    }

    node_destroy(bucket, key_destroy, val_destroy);
}



////////////////////////////////////// FUNCTIONS FOR RESIZING /////////////////////////////////////

// Rehash elements of table to table of new capacity, return true if successful
bool rehash(Hashtable table, size_t new_capacity)
{
    // Save old capacity and buckets and change old values
    size_t old_capacity = table->capacity;
    Node *old_buckets = table->buckets;

    table->capacity = new_capacity;
    table->threshold = table->capacity * table->load_factor;

    table->buckets = calloc(table->capacity, sizeof(Node));
    if (!table->buckets) return false;

    // Go through every bucket in old table
    for (size_t i = 0; i < old_capacity; i++)
    {
        Node cur_node = old_buckets[i];
        if (!cur_node) continue;

        Node next_node = cur_node->next;

        // Insert every node to new table
        while (next_node)
        {
            size_t index = cur_node->hash % table->capacity;
            table->buckets[index] = bucket_insert_node(table->buckets[index], cur_node);

            cur_node = next_node;
            next_node = cur_node->next; 
        }

        size_t index = cur_node->hash % table->capacity;
        table->buckets[index] = bucket_insert_node(table->buckets[index], cur_node);
    }

    free(old_buckets);
    return true;
}

// Check if table needs resizing, return false only if resizing was unsuccessful
bool hash_resize(Hashtable table)
{
    while (table->size >= table->threshold)
    {
        if (table->capacity >= MAX_PRIME)
            return rehash(table, table->capacity * 2);

        return rehash(table, hash_sizes[++table->cap_index]);
    }
    return true;
}


///////////////////////////////////////// MAIN FUNCTIONS ///////////////////////////////////////////


Hashtable hash_init(hashFunc key_hash, cmpFunc key_cmp, destroyFunc key_destroy, destroyFunc val_destroy)
{
    if (!key_hash || !key_cmp)
        return NULL;

    Hashtable table = malloc(sizeof(struct hashtable));
    if (!table) return NULL;

    table->cap_index = 0;
    table->capacity = hash_sizes[0];
    table->size = 0;
    table->load_factor = LOAD_FACTOR;
    table->threshold = table->capacity * table->load_factor;

    table->buckets = calloc(table->capacity, sizeof(Node));

    if (!table->buckets)
    {
        free(table); return NULL;
    }

    table->key_hash = key_hash;
    table->key_cmp = key_cmp;
    table->val_destroy = val_destroy;
    table->key_destroy = key_destroy;
    table->flag = OK;

    return table;
}

bool hash_empty(Hashtable table)
{
    return (table->size == 0);
}

size_t hash_size(Hashtable table)
{
    return table->size;
}

bool hash_insert(Hashtable table, void *key, void *value)
{
    // Resize table if necessary
    if (!hash_resize(table))
    {
        table->flag = ALLOC;
        return false;
    }

    // Get hash and index of key
    size_t hash = table->key_hash(key);
    size_t index = hash % table->capacity;

    // Check if key is already in table
    if (bucket_replace(table->buckets[index], key, value, table->key_cmp, table->key_destroy, table->val_destroy))
        return true;

    // If not, insert it at that index
    Node new_bucket = bucket_insert(table->buckets[index], key, value, hash);

    if (!new_bucket) 
    {
        table->flag = ALLOC;
        return false;
    }

    table->buckets[index] = new_bucket;
    table->size++;
    return true;
}

void *hash_search(Hashtable table, void *key)
{
    size_t index = table->key_hash(key) % table->capacity;
    Node node = bucket_search(table->buckets[index], key, table->key_cmp);
    
    if (!node) return NULL;
    return node->value;
}

bool hash_remove(Hashtable table, void *key)
{
    size_t index = table->key_hash(key) % table->capacity;
    bool found;

    table->buckets[index] = bucket_remove(table->buckets[index], key, table->key_cmp, table->key_destroy, table->val_destroy, &found);

    if (found) table->size--;
    return found;
}

bool hash_clear(Hashtable table)
{
    for (size_t i = 0; i < table->capacity; i++)
        bucket_destroy(table->buckets[i], table->key_destroy, table->val_destroy);

    free(table->buckets);

    table->cap_index = 0;
    table->capacity = hash_sizes[0];
    table->size = 0;
    table->threshold = table->capacity * table->load_factor;

    table->buckets = calloc(table->capacity, sizeof(Node));

    if (!table->buckets)
    {
        table->flag = ALLOC;
        return false;
    }

    return true;
}

void hash_destroy(Hashtable table)
{
    for (size_t i = 0; i < table->capacity; i++)
        bucket_destroy(table->buckets[i], table->key_destroy, table->val_destroy);

    free(table->buckets);
    free(table);
}

double hash_load_factor(Hashtable table)
{
    return table->load_factor;
}

void hash_set_key_destroy(Hashtable table, destroyFunc key_destroy)
{
    table->key_destroy = key_destroy;
}

void hash_set_val_destroy(Hashtable table, destroyFunc val_destroy)
{
    table->val_destroy = val_destroy;
}

bool hash_set_load_factor(Hashtable table, double load_factor)
{
    table->load_factor = load_factor;
    table->threshold = table->capacity * table->load_factor;

    return hash_resize(table);
}

int hash_flag(Hashtable table)
{
    return table->flag;
}