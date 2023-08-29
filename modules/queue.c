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
    Queue queue2 = queue_init(queue->destroy);
    ERR_ALLOC(queue, queue2);

    Node cur_node = queue->head;

    // Enqueue every element of old queue to new queue
    while(cur_node)
    {
        void *data = copy ? copy(cur_node->data) : cur_node->data;

        // In case of failed allocation, free memory allocated for new queue
        if (!data || !queue_enqueue(queue2, data))
        {
            queue->flag = ALLOC;
            queue_destroy(queue2);
            return NULL;
        }

        cur_node = cur_node->next;
    }

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