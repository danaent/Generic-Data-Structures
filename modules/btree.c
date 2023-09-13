#include "../include/btree.h"
#include "../include/flags.h"
#include <string.h>


#define ELEM_ALREADY_IN_NODE -1

typedef enum insert_status {INSERTED, NOT_INSERTED, ALLOC_FAIL} insert_status;

typedef struct node *Node;

struct node
{
    Node parent;     // Parent of node
    int count;       // Number of elements in node
    void **data;     // Array that stores every element in node
    Node *children;  // Array that stores pointers to the node's children
};

struct btree
{
    Node root;       // Root of the tree
    size_t size;     // Number of elements in tree
    int min;         // Min number of elements each node can store (a-1)
    int max;         // Max number of elements each node can store (b-1)
    cmpFunc cmp;
    destroyFunc destroy;
    int flag;
};


// Prototype
static void *fix_underflow(Node node, int del_index, int max, cmpFunc cmp);


//////////////////////////////////// FUNCTIONS FOR NODES ////////////////////////////////////////

// Create and return a new node that can store max elements
static Node node_new(int max)
{
    Node new_node = malloc(sizeof(struct node));
    if (!new_node) return NULL;

    new_node->parent = NULL;
    new_node->count = 0;

    new_node->data = calloc(max, sizeof(void *));
    new_node->children = calloc(max + 1, sizeof(Node));

    if (!new_node->data || !new_node->children)
    {
        free(new_node); 
        free(new_node->data);
        free(new_node->children);
        return NULL;
    }

    return new_node;
}

// Return true if node is the root
static inline bool node_is_root(Node node)
{
    return node->parent == NULL;
}

// Return true if node is leaf (does not have children)
static inline bool node_is_leaf(Node node)
{
    return node->children[0] == NULL;
}

// Return true if inserting an element causes an overflow
static inline bool node_overflow(Node node, int max)
{
    // If node is NULL return true
    return node ? node->count == max : true;
}

// Return true if removing an element causes an underflow
static inline bool node_underflow(Node node, int min)
{
    return node->count == min;
}

// Free memory allocated for node
static inline void node_destroy(Node node)
{
    free(node->data);
    free(node->children);
    free(node);
}

// Return the index where data should be inserted in node, or ELEM_ALREADY_IN_NODE if node already contains data
static int node_new_index(Node node, void *data, cmpFunc cmp)
{
    for (int i = 0; i < node->count; i++)
    {
        int cmp_val = cmp(data, node->data[i]);

        if (cmp_val < 0)
            return i;

        if (!cmp_val)
            return ELEM_ALREADY_IN_NODE;
    }

    return node->count;
}

// Insert data into node at index
static void node_insert_data(Node node, void *data, int index)
{
    // Shift other elements to fit data
    memcpy(node->data + index + 1, node->data + index, (node->count - index) * sizeof(void *));

    // Shift children
    if (!node_is_leaf(node))
        memcpy(node->children + index + 1, node->children + index, (node->count - index + 1) * sizeof(Node));

    // Insert data and increase count
    node->data[index] = data;
    node->count++;
}

// Remove and return data at index of node
static void *node_remove_data(Node node, int index)
{
    // Save data and shift elements to remove it
    void *data = node->data[index];
    memcpy(node->data + index, node->data + index + 1, (node->count-- - index - 1) * sizeof(void *));
    return data;
}

// Swap element at index of node with its predecessor in the tree (located in a leaf node), return that leaf node
static Node node_swap_with_leaf(Node node, int index)
{
    // Search left subtree of element at index
    Node leaf_node = node->children[index];

    // Travel to the rightmost leaf node of that subtree
    while (!node_is_leaf(leaf_node))
        leaf_node = leaf_node->children[leaf_node->count];

    // Find rightmost element of leaf node and swap
    int leaf_index = leaf_node->count - 1;

    void *temp = leaf_node->data[leaf_index];
    leaf_node->data[leaf_index] = node->data[index];
    node->data[index] = temp;

    return leaf_node;
}



//////////////////////////////////// BALANCING FUNCTIONS ////////////////////////////////////////

// Perform split operations upwards starting from node to fit data into tree, return a status code
static insert_status split(Node node, void *data, cmpFunc cmp, int max)
{
    Node left_node = NULL;              // Nodes that will be the children left and right of the new element
    Node right_node = NULL;

    int middle_index = node->count / 2; // Right-based middle index
    int new_index;                      // Index where new element will be inserted

    // Work upwards splitting nodes to fix overflows
    while(node_overflow(node, max))
    {
        // If the root overflows
        if (node == NULL)
        {
            // Create a new root and add new element
            node = node_new(max);
            if (!node) return ALLOC_FAIL;

            node->data[node->count++] = data;
            left_node->parent = right_node->parent = node;
            node->children[0] = left_node;
            node->children[1] = right_node;
            return INSERTED;
        }

        // Split node in two nodes, current node is the left part
        // Create a new node for the right part
        Node split_left = node;
        Node split_right = node_new(max);

        // Half of the elements go to the right part of the node
        split_right->count = split_left->count - middle_index;
        split_right->parent = split_left->parent;

        // Find index of new element
        new_index = node_new_index(split_left, data, cmp);

        // Now the middle element is propagated upwards

        // If new element's position is in the middle, it is propagated upwards
        if (new_index == middle_index)
        {
            // Insert half of left part's elements into right part
            memcpy(split_right->data, split_left->data + middle_index, split_right->count * sizeof(void *));

            if (!node_is_leaf(split_left))
            {  
                // Add new children to left and right part
                split_left->children[middle_index] = left_node;
                split_right->children[0] = right_node;

                // Move half of left part's children to right part
                memcpy(split_right->children + 1, split_left->children + middle_index + 1, split_right->count * sizeof(Node));

                for (int i = 0; i <= split_right->count; i++)
                    split_right->children[i]->parent = split_right;
            }
        }

        // If new element's position is before the middle, it is added to the left part
        else if (new_index < middle_index)
        {
            // Insert half of left part's elements into right part
            memcpy(split_right->data, split_left->data + middle_index, split_right->count * sizeof(void *));

            // Shift elements of left part to add new element
            memcpy(split_left->data + new_index + 1, split_left->data + new_index, (middle_index - new_index) * sizeof(void *));
            split_left->data[new_index] = data;

            // Middle element will be propagated upwards
            data = split_left->data[middle_index];

            if (!node_is_leaf(split_left))
            {
                // Move half of left part's children to right part
                memcpy(split_right->children, split_left->children + middle_index, (split_right->count + 1) * sizeof(Node));

                for (int i = 0; i <= split_right->count; i++)
                    split_right->children[i]->parent = split_right;

                // Shift left part's children to add new children
                memcpy(split_left->children + new_index + 2, split_left->children + new_index + 1, (middle_index - new_index - 1) * sizeof(Node));
                split_left->children[new_index] = left_node;
                split_left->children[new_index + 1] = right_node;
            }
        }

        // If new element's position is after the middle, it is added to the right part
        else
        {
            // Insert half of left part's elements into right part and include the new element
            memcpy(split_right->data, split_left->data + middle_index + 1, (new_index - middle_index - 1) * sizeof(void *));
            split_right->data[new_index - middle_index - 1] = data;
            memcpy(split_right->data + new_index - middle_index, split_left->data + new_index, (split_left->count - new_index) * sizeof(void *));

            // Middle element will be propagated upwards
            data = split_left->data[middle_index];

            if (!node_is_leaf(split_left))
            {
                // Move half of left part's children to right part and add new children
                memcpy(split_right->children, split_left->children + middle_index + 1, (new_index - middle_index - 1) * sizeof(Node));
                split_right->children[new_index - middle_index - 1] = left_node;
                split_right->children[new_index - middle_index] = right_node;
                memcpy(split_right->children + new_index - middle_index + 1, split_left->children + new_index + 1, (split_left->count - new_index) * sizeof(Node));

                for (int i = 0; i <= split_right->count; i++)
                    split_right->children[i]->parent = split_right;
            }
        }

        // Left part's elements have been halved
        split_left->count = middle_index;

        // New split nodes will be added to the parent along with the propagated element
        left_node = split_left;
        right_node = split_right;
        node = node->parent;
    }

    // Insert new element into node
    new_index = node_new_index(node, data, cmp);
    node_insert_data(node, data, new_index);
    node->children[new_index] = left_node;
    node->children[new_index + 1] = right_node;

    return INSERTED;
}

// Perform a transfer from the child at left_index of parent_node to its right sibling and remove element at del_index of right sibling, return that element
static void *transfer_left_to_right(Node parent_node, int left_index, int del_index)
{
    // Indexes of left and right nodes
    int right_index = left_index + 1;

    // Left and right nodes
    Node left_node = parent_node->children[left_index];
    Node right_node = parent_node->children[right_index];

    // Save element that will be deleted from right node
    void *del_data = right_node->data[del_index];

    // Move element from parent node to right node
    memcpy(right_node->data + 1, right_node->data, del_index * sizeof(void *));
    right_node->data[0] = parent_node->data[left_index];

    if (!node_is_leaf(right_node))
    {
        // Move rightmost child of left node to right node
        memcpy(right_node->children + 1, right_node->children, (del_index + 1) * sizeof(Node));
        right_node->children[0] = left_node->children[left_node->count];
        right_node->children[0]->parent = right_node;
    }

    // Move rightmost element of left node to parent node
    parent_node->data[left_index] = left_node->data[--left_node->count];
    return del_data;
}

// Perform a transfer to the child at left_index of parent_node from its right sibling and remove element at del_index of left sibling, return that element
static void *transfer_right_to_left(Node parent_node, int left_index, int del_index)
{
    // Indexes of left and right nodes
    int right_index = left_index + 1;

    // Left and right nodes
    Node left_node = parent_node->children[left_index];
    Node right_node = parent_node->children[right_index];

    // Save element that will be deleted from left node
    void *del_data = left_node->data[del_index];

    // Remove that element and move element from parent to left node
    memcpy(left_node->data + del_index, left_node->data + del_index + 1, (left_node->count - del_index - 1) * sizeof(void *));
    left_node->data[left_node->count - 1] = parent_node->data[left_index];
    
    if (!node_is_leaf(left_node))
    {
        // Move leftmost child of right node to left node
        memcpy(left_node->children + del_index + 1, left_node->children + del_index + 2, (left_node->count - del_index - 1) * sizeof(Node));
        left_node->children[left_node->count] = right_node->children[0];
        left_node->children[left_node->count]->parent = left_node;

        memcpy(right_node->children, right_node->children + 1, right_node->count * sizeof(Node));
    }

    // Move leftmost element of right node to parent
    parent_node->data[left_index] = right_node->data[0];
    memcpy(right_node->data, right_node->data + 1, (--right_node->count) * sizeof(void *));

    return del_data;
}

// Merges child at left_index of parent_node with its right sibling in order to remove element at del_index of child indicated by del_from_left, return that element
static void *fusion(Node parent_node, int left_index, int del_index, bool del_from_left, int max, cmpFunc cmp)
{
    // Indexes of children left and right of element to be deleted
    int right_index = left_index + 1;

    // Children left and right of element to be deleted
    Node left_node = parent_node->children[left_index];
    Node right_node = parent_node->children[right_index];

    // Save element from parent node that will be inserted into fused node 
    void *parent_data = parent_node->data[left_index];
    void *del_data; 

    // If there is an underflow in parent node, fix it
    if (node_underflow(parent_node, max) && !node_is_root(parent_node))
    { 
        fix_underflow(parent_node, left_index, max, cmp);
    }
    // If not remove element from parent that will be inserted in fused node
    else
    {
        memcpy(parent_node->data + left_index, parent_node->data + right_index, (parent_node->count - right_index) * sizeof(void *));

        if (!node_is_leaf(parent_node))
            memcpy(parent_node->children + right_index, parent_node->children + right_index + 1, (parent_node->count-- - right_index) * sizeof(Node));
    }

    // If element is to be deleted from left child
    if (del_from_left)
    {
        // Save element
        del_data = left_node->data[del_index];

        // Fuse left and right nodes in left node, adding element from parent too
        memcpy(left_node->data + del_index, left_node->data + del_index + 1, (left_node->count - del_index - 1) * sizeof(void *));
        memcpy(left_node->data + left_node->count, right_node->data, right_node->count * sizeof(void *));
        left_node->data[left_node->count - 1] = parent_data;

        if (!node_is_leaf(left_node))
        {
            // Move children of both nodes accordingly
            memcpy(left_node->children + del_index + 1, left_node->children + del_index + 2, (left_node->count - del_index - 1) * sizeof(Node));
            memcpy(left_node->children + left_node->count, right_node->children, (right_node->count + 1) * sizeof(Node));

            for (int i = left_node->count; i <= left_node->count + right_node->count; i++)
                left_node->children[i]->parent = left_node;
        }
    }
    // Same if element is deleted from right child
    else
    {
        del_data = right_node->data[del_index];

        memcpy(left_node->data + left_node->count + 1, right_node->data, del_index * sizeof(void *));
        memcpy(left_node->data + left_node->count + del_index + 1, right_node->data + del_index + 1, (right_node->count - del_index - 1) * sizeof(void *));
    
        left_node->data[left_node->count] = parent_data;

        if (!node_is_leaf(left_node))
        {
            memcpy(left_node->children + left_node->count + 1, right_node->children, (del_index + 1) * sizeof(Node));
            memcpy(left_node->children + left_node->count + del_index + 2, right_node->children + del_index + 2, (right_node->count - del_index - 1) * sizeof(Node));

            for (int i = left_node->count + 1; i <= left_node->count + right_node->count; i++)
                left_node->children[i]->parent = left_node;
        }
    }

    // Fix fused node's count
    left_node->count += right_node->count;

    // Free memory for right node and return element to be deleted
    node_destroy(right_node);
    return del_data;

}

// Remove and return element at del_index of node, fixing underflows on the way
static void *fix_underflow(Node node, int del_index, int max, cmpFunc cmp)
{
    Node parent_node = node->parent;

    // Get child index of node in parent
    int child_index = -1;
    while (parent_node->children[++child_index] != node);

    // If a sibling has >a children, perform a transfer operation
    if (child_index > 0 && !node_underflow(parent_node->children[child_index - 1], max))
        return transfer_left_to_right(parent_node, child_index - 1, del_index);
    else if (child_index < parent_node->count && !node_underflow(parent_node->children[child_index + 1], max))
        return transfer_right_to_left(parent_node, child_index, del_index);

    // Otherwise perform a fusion with a sibling
    else if (child_index < parent_node->count)
        return fusion(parent_node, child_index, del_index, true, max, cmp);
    else
        return fusion(parent_node, child_index - 1, del_index, false, max, cmp);

}



////////////////////////////////// RECURSIVE TREE FUNCTIONS ///////////////////////////////////////

// Insert data in tree rooted at node, return a status code indicating success or failure
static insert_status node_insert(Node node, void *data, int max, cmpFunc cmp)
{
    // Get index where data should be placed
    int new_index = node_new_index(node, data, cmp);

    if(new_index == ELEM_ALREADY_IN_NODE)
        return NOT_INSERTED;

    // If node is leaf insert here
    if (node_is_leaf(node))
    {
        // If inserting results in an overflow, split node
        if(node_overflow(node, max))
            return split(node, data, cmp, max);

        node_insert_data(node, data, new_index);
        return INSERTED;
    }

    // If node isn't leaf, keep searching
    return node_insert(node->children[new_index], data, max, cmp);
}

// Search for data in tree rooted at node, return that data or NULL if not found 
static void *node_search(Node node, void *data, cmpFunc cmp)
{
    // If NULL node is reached, element is not in the tree
    if (!node)
        return NULL;

    // Search through its element
    int index;
    for (index = 0; index < node->count; index++)
    {
        int cmp_val = cmp(data, node->data[index]);

        // If found, return
        if (!cmp_val)
            return node->data[index];

        // If element with smaller priority is found, search in this subtree
        if (cmp_val < 0)
            break;
    }

    return node_search(node->children[index], data, cmp);
}

// Remove and return data from tree, NULL if data is not found
static void *node_remove(Node node, void *data, int min, cmpFunc cmp)
{
    // If NULL node is reached, element is not in the tree
    if (!node)
        return NULL;

    // Search for element in node
    int index;
    for (index = 0; index < node->count; index++)
    {
        int cmp_val = cmp(data, node->data[index]);

        if (!cmp_val)
            break;

        if (cmp_val < 0)
            return node_remove(node->children[index], data, min, cmp);
    }

    if (index == node->count)
        return node_remove(node->children[index], data, min, cmp);

    // If it's not a leaf node, swap with element in leaf node
    if (!node_is_leaf(node))
    {
        node = node_swap_with_leaf(node, index);
        index = node->count - 1;
    }

    // If no underflow occurs, remove data
    if (node_is_root(node) || !node_underflow(node, min))
        return node_remove_data(node, index);

    // Otherwise fix underflow
    return fix_underflow(node, index, min, cmp);;
}

// Free memory allocated for the tree rooted at node
static void node_destroy_all(Node node, destroyFunc destroy)
{
    if (!node)
        return;

    // Destroy every subtree
    for (int i = 0; i <= node->count; i++)
        node_destroy_all(node->children[i], destroy);

    // Destroy all elements
    if (destroy)
        for (int i = 0; i < node->count; i++)
            destroy(node->data[i]);

    node_destroy(node);
}

// Return element with minimum priority in tree rooted at node
static void *node_min(Node node)
{
    while (!node_is_leaf(node))
        node = node->children[node->count];

    return node->data[node->count-1];
}

// Return element with maximum priority in tree rooted at node
static void *node_max(Node node)
{
    while (!node_is_leaf(node))
        node = node->children[0];

    return node->data[0];
}

// Return a deep copy of tree rooted at node
static Node node_copy_deep(Node node, int max, copyFunc copy, bool *alloc_fail)
{
    // Create a copy of the old node
    Node node_copied = node_new(max);
    if (!node_copied) return NULL;

    node_copied->count = node->count;

    // If node has children
    if (node->children[0])
    {
        // Copy both its elements and its children
        for (int i = 0; i < node->count; i++)
        {
            node_copied->data[i] = copy(node->data[i]);
            node_copied->children[i] = node_copy_deep(node->children[i], max, copy, alloc_fail);

            if (!node_copied->data[i] || !node_copied->children[i])
            {
                *alloc_fail = true;
                return node_copied;
            }
            else 
                node_copied->children[i]->parent = node_copied;
        }

        node_copied->children[node->count] = node_copy_deep(node->children[node->count], max, copy, alloc_fail);

        if (!node_copied->children[node->count])
        {
            *alloc_fail = true;
            return node_copied;
        }
        else
            node_copied->children[node->count]->parent = node_copied;
    }
    else
    {
        // Only copy elements
        for (int i = 0; i < node->count; i++)
        {
            node_copied->data[i] = copy(node->data[i]);

            if (!node_copied->data[i])
            {
                *alloc_fail = true;
                return node_copied;
            }
        }
    }

    return node_copied;
}

// Return a shallow copy of tree rooted at node
static Node node_copy_shallow(Node node, int max, bool *alloc_fail)
{
    // Create a copy of the old node and copy its elements
    Node node_copied = node_new(max);
    if (!node_copied)
    {
        *alloc_fail = true; return NULL;
    }

    memcpy(node_copied->data, node->data, node->count * sizeof(void *));
    node_copied->count = node->count;

    // If node has children, visit and copy them
    if (node->children[0])
    {
        for (int i = 0; i <= node->count; i++)
        {
            node_copied->children[i] = node_copy_shallow(node->children[i], max, alloc_fail);

            if (!node_copied->children[i])
                return node_copied;
            else
                node_copied->children[i]->parent = node_copied;
        }
    }

    return node_copied;
}



///////////////////////////////////////// MAIN FUNCTIONS ///////////////////////////////////////////

BTree btree_init(int b, cmpFunc cmp, destroyFunc destroy)
{
    if (b < 3 || !cmp)
        return NULL;

    BTree btree = malloc(sizeof(struct btree));
    if (!btree) return NULL;

    btree->root = NULL;
    btree->size = 0;
    btree->max = b-1;
    btree->min = btree->max % 2 ? btree->max / 2 : btree->max / 2 - 1;
    btree->cmp = cmp;
    btree->destroy = destroy;
    btree->flag = OK;

    return btree;
}

bool btree_empty(BTree btree)
{
    return (btree->size == 0);
}

size_t btree_size(BTree btree)
{
    return btree->size;
}

bool btree_insert(BTree btree, void *data)
{
    // If tree is empty add a root node
    if (!btree->root)
    {
        btree->root = node_new(btree->max);
        ERR_ALLOC(btree, btree->root)

        btree->root->data[btree->root->count++] = data;
        btree->size++;
        return true;
    }

    // Otherwise insert a new node
    insert_status status = node_insert(btree->root, data, btree->max, btree->cmp);

    // If node was successfully inserted
    if (status == INSERTED)
    {
        // Increase size
        btree->size++;

        // If there's a new root, change root
        if (btree->root->parent)
            btree->root = btree->root->parent;

        return true;
    }
    else if (status == ALLOC_FAIL) btree->flag = ALLOC;

    return false;
}

void *btree_search(BTree btree, void *data)
{
    return node_search(btree->root, data, btree->cmp);
}

bool btree_remove(BTree btree, void *data)
{
    // Remove and return data from tree
    data = node_remove(btree->root, data, btree->min, btree->cmp);

    // If data was removed
    if (data)
    {
        btree->size--;
        
        // If tree is empty remove root
        if (!btree->size)
        {
            node_destroy(btree->root); btree->root = NULL;
        }

        // Else if root is empty but tree isn't, change root
        else if (!btree->root->count)
        {
            Node root = btree->root;

            // Previous operations have left the root with a single child
            btree->root = root->children[0];
            btree->root->parent = NULL;

            node_destroy(root);
        }

        // Free memory allocated for data
        if (btree->destroy)
            btree->destroy(data);

        return true;
    }

    return false;
}

void *btree_min(BTree btree)
{
    ERR_EMPTY(btree)
    return node_min(btree->root);
}

void *btree_max(BTree btree)
{
    ERR_EMPTY(btree)
    return node_max(btree->root);
}

BTree btree_copy(BTree btree, copyFunc copy)
{
    // Initialize new tree
    BTree btree2 = btree_init(btree->max + 1, btree->cmp, btree->destroy);
    ERR_ALLOC(btree, btree2)

    btree2->size = btree->size;
    bool alloc_fail = false;

    // Copy old tree
    if (copy)
        btree2->root = node_copy_deep(btree->root, btree->max, copy, &alloc_fail);
    else
        btree2->root = node_copy_shallow(btree->root, btree->max, &alloc_fail);

    if (alloc_fail)
    {
        btree_destroy(btree2);
        btree->flag = ALLOC;
        return NULL;
    }

    return btree2;
}

void btree_clear(BTree btree)
{
    // Destroy every node
    node_destroy_all(btree->root, btree->destroy);

    // Re-initialize tree
    btree->root = NULL;
    btree->size = 0;
}

void btree_destroy(BTree btree)
{
    node_destroy_all(btree->root, btree->destroy);
    free(btree);
}

void btree_set_destroy(BTree btree, destroyFunc destroy)
{
    btree->destroy = destroy;
}

int btree_flag(BTree btree)
{
    return btree->flag;
}
