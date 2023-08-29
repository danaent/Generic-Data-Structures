#include "../include/list.h"
#include "../include/flags.h"

typedef struct node* Node;

struct node
{
    void *data;          // Element stored in node
    Node next;           // Pointer to next element
    Node prev;           // Pointer to previous element
};

struct list
{
    Node head;           // Pointer to first node of list
    Node tail;           // Pointer to last node of list
    size_t size;         // Number of elements in list
    int flag;
    destroyFunc destroy;
};


//////////////////////////////////// FUNCTIONS FOR NODES ////////////////////////////////////////

// Create and return a new node with data or NULL in case of failure
static inline Node node_new(void *data)
{
    Node node = malloc(sizeof(struct node));
    if (!node) return NULL;

    node->data = data;
    return node;
}

// Insert new node before a node in the middle of the list
static inline void node_insert_before(Node cur_node, Node new_node)
{
    // Set new node's prev and next pointers
    new_node->next = cur_node;
    new_node->prev = cur_node->prev;

    // Set pointers of nodes before and after it
    cur_node->prev->next = new_node;
    cur_node->prev = new_node;
}

// Insert new node after a node in the middle of the list
static inline void node_insert_after(Node cur_node, Node new_node)
{
    new_node->prev = cur_node;
    new_node->next = cur_node->next;

    cur_node->next->prev = new_node;
    cur_node->next = new_node;
}

// Remove a node in the middle of the list and return its data
static inline void *node_remove(Node node)
{
    // Save data of node
    void *data = node->data;

    // Set pointers of previous and next node
    node->prev->next = node->next;
    node->next->prev = node->prev;

    // Free memory for node and return data
    free(node);
    return data;
}

// Return node with data in list or NULL if it does not exist
static inline Node node_search(Node head, void *data, cmpFunc cmp)
{
    while(head && cmp(data, head->data))
        head = head->next;
    
    return head;
}

// Return node at index of list
static inline Node list_node_index(List list, size_t index)
{
    Node cur_node;

    // If index is closer to start, traverse list from start
    // If it's closer to the end, traverse list from end
    if (index * 2 < list->size)
    {
        cur_node = list->head;

        for (size_t i = 0; i < index; i++)
            cur_node = cur_node->next;
    }
    else
    {
        cur_node = list->tail;

        for (size_t i = list->size - 1; i > index; i--)
            cur_node = cur_node->prev;
    }

    return cur_node;
}



///////////////////////////////////////// MAIN FUNCTIONS ///////////////////////////////////////////

List list_init(destroyFunc destroy)
{
    List list = malloc(sizeof(struct list));
    if (!list) return NULL;

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->destroy = destroy;
    list->flag = OK;
    
    return list;
}

bool list_empty(List list)
{
    return (list->size == 0);
}

size_t list_size(List list)
{
    return list->size;
}

void *list_get(List list, void *data, cmpFunc cmp)
{
    ERR_FUNC(list, cmp)
    
    // Traverse list looking for data
    Node cur_node = node_search(list->head, data, cmp);

    // If end of list is reached, data is not found
    if (!cur_node)
        return NULL;

    return cur_node->data;
}

void *list_get_first(List list)
{
    ERR_EMPTY(list)
    return list->head->data;
}

void *list_get_last(List list)
{
    ERR_EMPTY(list)
    return list->tail->data;
}

void *list_get_at(List list, size_t index)
{
    ERR_BOUNDS(list, index, list->size)

    // Find node at index and return its element
    Node cur_node = list_node_index(list, index);
    return cur_node->data;
}

bool list_prepend(List list, void *data)
{
    // Allocate memory for new node
    Node new_node = node_new(data);
    ERR_ALLOC(list, new_node)

    // Set next and prev nodes
    new_node->prev = NULL;
    new_node->next = list->head;

    // Change head
    list->head = new_node;

    if (!list->size)        // If list is empty, new node is both head and tail
        list->tail = new_node;
    else                    // If not, set next node's prev pointer to new node
        new_node->next->prev = new_node;

    // Increase size
    list->size++;
    return true;
}

bool list_append(List list, void *data)
{
    // Allocate memory for new node
    Node new_node = node_new(data);
    ERR_ALLOC(list, new_node)

    // Set next and prev nodes
    new_node->prev = list->tail;
    new_node->next = NULL;

    // Change tail
    list->tail = new_node;

    if (!list->size)        // If list is empty, new node is both head and tail
        list->head = new_node;
    else                    // If not, set next node's prev pointer to new node
        new_node->prev->next = new_node;

    // Increase size
    list->size++;
    return true;
}

bool list_insert(List list, void *data, size_t index)
{
    ERR_BOUNDS(list, index, list->size)

    // Call prepend if index is at start
    if (!index) return list_prepend(list, data);

    // Allocate memory for new node
    Node node = node_new(data);
    ERR_ALLOC(list, node)

    // Find node at index and insert new node before it
    Node cur_node = list_node_index(list, index);
    node_insert_before(cur_node, node);
    list->size++;

    return true;
}

bool list_remove(List list, void *data, cmpFunc cmp)
{
    // Pop element from list
    data = list_pop(list, data, cmp);

    // If element was not found, return false
    if (!data)
        return false;

    // Destroy element
    if (list->destroy)
        list->destroy(data);

    return true;
}

bool list_remove_all(List list, void *data, cmpFunc cmp)
{
    ERR_FUNC(list, cmp)
    Node cur_node;

    // While first element is equal to data, remove it
    while((cur_node = list->head) && !cmp(data, cur_node->data))
        list_remove_first(list);
    
    // Repeat for last element
    while((cur_node = list->tail) && !cmp(data, cur_node->data))
        list_remove_last(list);

    // Search list for elements equal to data and remove them
    cur_node = list->head;

    while(cur_node)
    { 
        if (!cmp(data, cur_node->data))
        {
            Node temp_node = cur_node;
            cur_node = cur_node->prev;

            void *data = node_remove(temp_node);
            if (list->destroy) list->destroy(data);
            list->size--;
        }

        cur_node = cur_node->next;
    }

    return true;
}

bool list_remove_first(List list)
{
    void *data = list_pop_first(list);

    if (!data)
        return false;

    if (list->destroy)
        list->destroy(data);

    return true;
}

bool list_remove_last(List list)
{
    void *data = list_pop_last(list);

    if (!data)
        return false;

    if (list->destroy)
        list->destroy(data);

    return true;
}

bool list_remove_at(List list, size_t index)
{
    void *data = list_pop_at(list, index);

    if (!data)
        return false;

    if (list->destroy)
        list->destroy(data);

    return true;
}

void *list_pop(List list, void *data, cmpFunc cmp)
{
    ERR_EMPTY(list)
    ERR_FUNC(list, cmp)

    // Search list for element
    Node cur_node = node_search(list->head, data, cmp);

    // If element is at start or end, call functions
    if (cur_node == list->head)
        return list_pop_first(list);

    if (cur_node == list->tail)
        return list_pop_last(list);

    // If element is not found, flag list
    if (!cur_node)
    {
        list->flag = ARG;
        return NULL;
    }

    // Remove node and return data
    data = node_remove(cur_node);
    list->size--;

    return data;
}

void *list_pop_first(List list)
{
    ERR_EMPTY(list)

    // Save head node in temp variable and move head to next node
    Node temp_node = list->head;
    list->head = temp_node->next;

    if (!--list->size)       // If list is now empty, set tail to NULL
        list->tail = NULL;
    else                     // If not, set head's prev node to NULL
        list->head->prev = NULL;

    // Free node and return its data
    void *data = temp_node->data;
    free(temp_node);

    return data;
}

void *list_pop_last(List list)
{
    ERR_EMPTY(list)

    // Save tail node in temp variable and move tail to previous node
    Node node = list->tail;
    list->tail = node->prev;

    if (!--list->size)       // If list is now empty, set head to NULL
        list->head = NULL;
    else                     // If not, set tail's next node to NULL
        list->tail->next = NULL;

    // Free node and return its data
    void *data = node->data;
    free(node);

    return data;
}

void *list_pop_at(List list, size_t index)
{
    ERR_BOUNDS(list, index, list->size)

    if (index == 0)
        return list_pop_first(list);

    if (index == list->size - 1)
        return list_pop_last(list);

    Node cur_node = list_node_index(list, index);
    void *data = node_remove(cur_node);

    list->size--;
    return data;
}

size_t list_count(List list, void *data, cmpFunc cmp)
{
    ERR_FUNC(list, cmp)

    Node cur_node = list->head;
    size_t count = 0;

    // Traverse list
    while(cur_node)
    {
        if (!cmp(data, cur_node->data)) // Increase count when data is encountered
            count++;

        cur_node = cur_node->next;
    }

    return count;
}

bool list_contains(List list, void *data, cmpFunc cmp)
{
    ERR_FUNC(list, cmp)
    Node cur_node = node_search(list->head, data, cmp);
    return (cur_node != NULL);
}

void list_reverse(List list)
{
    // If list is empty, return
    if (!list->size)
        return;

    // Switch head and tail
    Node cur_node = list->head;
    list->head = list->tail;
    list->tail = cur_node;

    // Traverse list and switch next and prev pointers
    while(cur_node)
    {
        Node next_node = cur_node->next;
        cur_node->next = cur_node->prev;
        cur_node->prev = next_node;

        cur_node = next_node;
    }
}

bool list_trim(List list, size_t start, size_t end)
{
    size_t init_size = list->size;

    for (size_t i = 0; i < start; i++)
        list_remove_first(list);

    for (size_t i = init_size; i > end; i--)
        list_remove_last(list);

    return true;
}


///////////////////////////////// FUNCTIONS FOR MERGE SORT //////////////////////////////////////

// Return middle node of list using the tortoise and the hare approach
static Node node_middle(Node head)
{
    Node slow = head;
    Node fast = head->next;

    while (fast && fast->next)
    {
        slow = slow->next;
        fast = fast->next->next;
    }

    return slow;
}

// Merge two sorted lists
static Node node_merge(Node head1, Node head2, cmpFunc cmp)
{
    // Merged list head and tail
    Node new_head = NULL;
    Node new_tail = NULL;

    // While neither list is empty
    while(head1 && head2)
    {
        Node next_node = NULL;

        // Next node of new list is the head whose element has the higher priority
        if (cmp(head1->data, head2->data) <= 0)
        {
            next_node = head1;
            head1 = head1->next;
        }
        else
        {
            next_node = head2;
            head2 = head2->next;
        }

        // If new list isn't empty, set tail pointers
        if (new_tail)
        {
            new_tail->next = next_node;
            next_node->prev = new_tail;
            new_tail = next_node;
        }
        // If it is, tail is equal to head
        else
        {
            new_head = new_tail = next_node;
            next_node->next = next_node->prev = NULL;
        }
    }

    // If first list hasn't been emptied, add it to the end of merged list
    if (head1) 
    {
        new_tail->next = head1;
        head1->prev = new_tail;
    }
    // Otherwise do the same with the second list
    else
    {
        new_tail->next = head2;
        head2->prev = new_tail;
    }

    return new_head;
}

// Recursively sort a list with merge sort
static Node node_merge_sort(Node head, cmpFunc cmp)
{
    // If list has one element or fewer, it doesn't need sorting
    if (!head || !head->next)
        return head;

    // Get middle node and split list in two
    Node middle_node = node_middle(head);
    Node head2 = middle_node->next;
    middle_node->next = NULL;
    head2->prev = NULL;
    
    // Sort each list
    head = node_merge_sort(head, cmp);
    head2 = node_merge_sort(head2, cmp);

    // Merge the two lists
    return node_merge(head, head2, cmp);
}

bool list_sort(List list, cmpFunc cmp)
{
    ERR_FUNC(list, cmp)

    // List is already sorted
    if (list->size <= 1)
        return true;

    // Sort list and get new head
    list->head = node_merge_sort(list->head, cmp);

    // Move pointer to find new tail
    while (list->tail->next)
        list->tail = list->tail->next;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////


bool list_insert_sorted(List list, void *data, cmpFunc cmp)
{
    ERR_FUNC(list, cmp)

    // Traverse list until element that has a higher than or equal priority to data is found
    Node cur_node = list->head;

    while(cur_node && cmp(data, cur_node->data) > 0)
        cur_node = cur_node->next;

    // If element is the first, prepend data
    if (cur_node == list->head)
        return list_prepend(list, data);

    // If end of list is reached, append data
    if (!cur_node)
        return list_append(list, data);

    // Create a new node and insert it before current
    Node new_node = node_new(data);
    ERR_ALLOC(list, new_node)

    node_insert_before(cur_node, new_node);
    list->size++;

    return true;
}

List list_copy(List list, copyFunc copy)
{
    // Initialize new list
    List list2 = list_init(list->destroy);
    ERR_ALLOC(list, list2)

    Node cur_node = list->head;

    // Append every element of old list to new list
    while(cur_node)
    {
        void *data = copy ? copy(cur_node->data) : cur_node->data;

        // In case of failed allocation, free memory allocated for new list
        if (!data || !list_append(list2, data))
        {
            list->flag = ALLOC;
            list_destroy(list2);
            return NULL;
        }

        cur_node = cur_node->next;
    }

    return list2;
}

void list_clear(List list)
{
    while(list->size) list_remove_first(list);
}

void list_destroy(List list)
{
    list_clear(list);
    free(list);
}

void list_set_destroy(List list, destroyFunc destroy)
{
    list->destroy = destroy;
}

int list_flag(List list)
{
    return list->flag;
}