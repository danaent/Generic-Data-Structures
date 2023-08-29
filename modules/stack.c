#include "../include/stack.h"
#include "../include/flags.h"

typedef struct node* Node;

struct node
{
    void *data;          // Element stored in node
    Node next;           // Pointer to next element
};

struct stack
{
    Node top;            // Pointer to first node of list
    size_t size;         // Number of elements in stack
    destroyFunc destroy;
    int flag;
};

Stack stack_init(destroyFunc destroy)
{
    Stack stack = malloc(sizeof(struct stack));
    if (!stack) return NULL;

    stack->top = NULL;
    stack->size = 0;
    stack->destroy = destroy;
    stack->flag = OK;

    return stack;
}

bool stack_empty(Stack stack)
{
    return (stack->size == 0);
}

size_t stack_size(Stack stack)
{
    return stack->size;
}

bool stack_push(Stack stack, void *data)
{
    // Allocate memory for new node
    Node new_node = malloc(sizeof(struct node));
    ERR_ALLOC(stack, new_node)

    // Add new node before top and change top
    new_node->data = data;
    new_node->next = stack->top;
    stack->top = new_node;

    stack->size++;
    return true;
}

void *stack_peek(Stack stack)
{
    ERR_EMPTY(stack)
    return stack->top->data;
}

void *stack_pop(Stack stack)
{
    ERR_EMPTY(stack)

    // Save top node's element and top node in temporary variables
    void *data = stack->top->data;
    Node temp_node = stack->top;
    
    // Change top and free memory for old top
    stack->top = stack->top->next;
    free(temp_node);
    stack->size--;

    // Return element
    return data;
}

bool stack_contains(Stack stack, void *data, cmpFunc cmp)
{
    ERR_FUNC(stack, cmp)

    // If stack is empty it does not contain element
    if (!stack->size)
        return false;

    Node cur_node = stack->top;

    // Iterate over stack to look for element
    while (cur_node)
    {
        if (!cmp(cur_node->data, data))
            return true;

        cur_node = cur_node->next;
    }

    return false;
}

Stack stack_copy(Stack stack, copyFunc copy)
{
    // Initialize new stack
    Stack stack2 = malloc(sizeof(struct stack));
    ERR_ALLOC(stack, stack2)

    stack2->top = NULL;
    stack2->size = stack->size;
    stack2->destroy = stack->destroy;

    // If stack is empty return copy now
    if (!stack->size) return stack2;

    // Otherwise allocate memory for top node
    stack2->top = malloc(sizeof(struct node));

    if (!stack2->top)
    {
        stack->flag = ALLOC;
        free(stack2); return NULL;
    }

    Node last_copied = stack2->top; // Iterator for new stack
    Node cur_node = stack->top->next; // Iterator for old stack

    // Copy old stack's top node
    stack2->top->data = copy ? copy(stack->top->data) : stack->top->data;

    if (!stack2->top->data)
    {
        stack->flag = ALLOC;
        stack2->top->next = NULL;
        stack_destroy(stack2);
        return NULL;
    }

    // Iterate over old stack and copy every element
    while (cur_node)
    {
        Node new_node = malloc(sizeof(struct node));
        void *data = copy ? copy(cur_node->data) : cur_node->data;

        // In case of failed allocation, free memory allocated for new stack
        if (!data || !new_node)
        {
            stack->flag = ALLOC;
            last_copied->next = NULL;
            stack_destroy(stack2); 
            return NULL;
        }

        new_node->data = data;
        last_copied->next = new_node;

        last_copied = new_node;
        cur_node = cur_node->next;
    }

    // End stack in NULL
    last_copied->next = NULL;
    stack2->flag = OK;
    
    return stack2;
}

void stack_clear(Stack stack)
{
    if (stack->destroy)
    {
        // Free memory for every node in stack
        while (stack->top)
        {
            Node temp = stack->top;
            stack->top = stack->top->next;

            stack->destroy(temp->data);
            free(temp);
        }
    }
    else
    {
        while (stack->top)
        {
            Node temp = stack->top;
            stack->top = stack->top->next;
            free(temp);
        }
    }

    stack->size = 0;
}

void stack_destroy(Stack stack)
{
    stack_clear(stack);
    free(stack);
}

void stack_set_destroy(Stack stack, destroyFunc destroy)
{
    stack->destroy = destroy;
}

int stack_flag(Stack stack)
{
    return stack->flag;
}