#include "../include/queue.h"
#include "../include/flags.h"

typedef struct node* Node;

struct node
{
    void *data;          // Element stored in node
    Node next;           // Pointer to next element
};

struct queue
{
    Node head;           // Pointer to first node of list
    Node tail;           // Pointer to last node of list
    size_t size;         // Number of elements in queue
    destroyFunc destroy;
    int flag;
};

Queue queue_init(destroyFunc destroy)
{
    Queue queue = malloc(sizeof(struct queue));
    if (!queue) return NULL;

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;

    queue->destroy = destroy;
    queue->flag = OK;

    return queue;
}

bool queue_empty(Queue queue)
{
    return (queue->size == 0);
}

size_t queue_size(Queue queue)
{
    return queue->size;
}

bool queue_enqueue(Queue queue, void *data)
{
    // Allocate memory for new node
    Node new_node = malloc(sizeof(struct node));
    ERR_ALLOC(queue, new_node)

    // Add data to node
    new_node->data = data;
    new_node->next = NULL;

    // If queue is empty new node is both head and tail
    if (!queue->head)
    {
        queue->head = queue->tail = new_node;
        queue->size++;
        return true;
    }

    // Otherwise add new node after tail
    queue->tail->next = new_node;
    queue->tail = new_node;
    queue->size++;

    return true;
}

void *queue_dequeue(Queue queue)
{
    ERR_EMPTY(queue)

    // Save head's element and head in temporary variables
    void *data = queue->head->data;
    Node temp_node = queue->head;
    
    // Change head and free memory for old head
    queue->head = queue->head->next;
    free(temp_node);
    queue->size--;

    // If queue has been emptied, update tail
    if (!queue->size)
        queue->tail = NULL;

    // Return element
    return data;
}

void *queue_peek(Queue queue)
{
    ERR_EMPTY(queue)
    return queue->head->data;
}

bool queue_contains(Queue queue, void *data, cmpFunc cmp)
{
    ERR_FUNC(queue, cmp)

    // If queue is empty it does not contain element
    if (!queue->size)
        return false;

    Node cur_node = queue->head;

    // Iterate over queue to look for element
    while (cur_node != NULL)
    {
        if (!cmp(cur_node->data, data))
            return true;

        cur_node = cur_node->next;
    }

    return false;
}

Queue queue_copy(Queue queue, copyFunc copy)
{
    // Initialize new queue
    Queue queue2 = malloc(sizeof(struct queue));
    ERR_ALLOC(queue, queue2)

    queue2->head = NULL;
    queue2->tail = NULL;
    queue2->size = queue->size;
    queue2->destroy = queue->destroy;

    // If queue is empty return copy now
    if (!queue->size) return queue2;

    // Otherwise allocate memory for new head
    queue2->head = malloc(sizeof(struct node));

    if (!queue2->head)
    {
        queue->flag = ALLOC;
        free(queue2); return NULL;
    }

    Node last_copied = queue2->head; // Iterator for new queue
    Node cur_node = queue->head->next; // Iterator for old queue

    // If copy function is given, do a deep copy
    if (copy)
    {
        // Copy old queue's head node
        queue2->head->data = copy(queue->head->data);

        // Iterate over old queue and copy every element
        while (cur_node)
        {
            Node new_node = malloc(sizeof(struct node));
            
            // In case of failed allocation free all memory already allocated for new queue
            if (!new_node)
            {
                queue->flag = ALLOC;
                last_copied->next = NULL;
                queue2->tail = last_copied;
                queue_destroy(queue2);
                return NULL;
            }

            new_node->data = copy(cur_node->data);
            last_copied->next = new_node;

            last_copied = new_node;
            cur_node = cur_node->next;
        }
    }
    else
    {
        queue2->head->data = queue->head->data;

        while (cur_node)
        {
            Node new_node = malloc(sizeof(struct node));
            
            if (!new_node)
            {
                queue->flag = ALLOC;
                last_copied->next = NULL;
                queue2->tail = last_copied;
                queue_destroy(queue2);
                return NULL;
            }

            new_node->data = cur_node->data;
            last_copied->next = new_node;

            last_copied = new_node;
            cur_node = cur_node->next;

        }
    }

    // End queue in NULL and set tail pointer
    last_copied->next = NULL;
    queue2->tail = last_copied;
    queue2->flag = OK;

    return queue2;
}

void queue_clear(Queue queue)
{
    if (queue->destroy)
    {
        // Free memory for every node in stack
        while (queue->head)
        {
            Node temp = queue->head;
            queue->head = queue->head->next;

            queue->destroy(temp->data);
            free(temp);
        }
    }
    else
    {
        while (queue->head)
        {
            Node temp = queue->head;
            queue->head = queue->head->next;
            free(temp);
        }
    }

    queue->tail = NULL;
    queue->size = 0;
}

void queue_destroy(Queue queue)
{
    queue_clear(queue);
    free(queue);
}

void queue_set_destroy(Queue queue, destroyFunc destroy)
{
    queue->destroy = destroy;
}

int queue_flag(Queue queue)
{
    return queue->flag;
}