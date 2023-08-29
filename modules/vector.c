#include "../include/vector.h"
#include "../include/flags.h"
#include <string.h>

#define MIN_CAPACITY 64     // Default minimum capacity
#define EXP_FACTOR 2        // Default expansion factor

struct vector
{
    void **array;           // Resizable array
    size_t size;            // Number of elements in vector
    size_t capacity;        // Number of elements current array can hold
    size_t min_capacity;    // Minimum capacity array can shrink to
    double exp_factor;      // Expansion factor, array capacity is multiplied by this number when it grows in size
    size_t prev_cap;        // New capacity array will shrink to
    int flag;
    destroyFunc destroy;
};


///////////////////////////////////// STATIC FUNCTIONS //////////////////////////////////////////

// Grow vector if vector size has reached its capacity 
static inline bool vector_grow(Vector vector)
{
    if (vector->size == vector->capacity)
    {
        vector->capacity *= vector->exp_factor;
        void **new_array = realloc(vector->array, vector->capacity * sizeof(void *));

        if (!new_array) return false;
        vector->array = new_array;
    }
    return true;
}

// Shrink vector if size is much smaller than capacity and new capacity >= min capacity
static inline bool vector_shrink(Vector vector)
{
    while (vector->capacity >= 2 * vector->size * vector->exp_factor && vector->prev_cap >= vector->min_capacity)
    {
        vector->capacity = vector->prev_cap;
        vector->prev_cap = vector->capacity / vector->exp_factor;
        void **new_array = realloc(vector->array, vector->capacity * sizeof(void *));

        if (!new_array) return false;
        vector->array = new_array;
    }
    return true;
}

// Swap contents of memory addresses a and b
static inline void swap_elements(void **a, void **b)
{
    void *temp = *a;
    *a = *b;
    *b = temp;
}

/////////////////////////////////////////////////////////////////////////////////////////////////


Vector vector_init(destroyFunc destroy)
{
    Vector vector = malloc(sizeof(struct vector));
    if (!vector) return NULL;

    vector->array = calloc(MIN_CAPACITY, sizeof(void *));

    if (!vector->array)
    {
        free(vector); return NULL;
    }

    vector->size = 0;
    vector->capacity = vector->min_capacity = MIN_CAPACITY;
    vector->exp_factor = EXP_FACTOR;
    vector->prev_cap = vector->capacity / vector->exp_factor;

    vector->destroy = destroy;
    vector->flag = OK;

    return vector;
}

Vector vector_init_custom(size_t min_capacity, double exp_factor, destroyFunc destroy)
{
    if (min_capacity < 1 || exp_factor <= 1) return NULL;

    Vector vector = malloc(sizeof(struct vector));
    if (!vector) return NULL;

    vector->array = calloc(min_capacity, sizeof(void *));

    if (!vector->array)
    {
        free(vector); return NULL;
    }

    vector->size = 0;
    vector->capacity = vector->min_capacity = min_capacity;
    vector->exp_factor = exp_factor;
    vector->prev_cap = vector->capacity / vector->exp_factor;

    vector->destroy = destroy;
    vector->flag = OK;

    return vector;
}

bool vector_empty(Vector vector)
{
    return (vector->size == 0);
}

size_t vector_size(Vector vector)
{
    return vector->size;
}

void *vector_get_first(Vector vector)
{
    return vector_get_at(vector, 0);
}

void *vector_get_last(Vector vector)
{
    return vector_get_at(vector, vector->size-1);
}

void *vector_get_at(Vector vector, size_t index)
{
    ERR_EMPTY(vector)
    ERR_BOUNDS(vector, index, vector->size)
    return vector->array[index];
}

bool vector_set_first(Vector vector, void *data)
{
    return vector_set_at(vector, data, 0);
}

bool vector_set_last(Vector vector, void *data)
{
    return vector_set_at(vector, data, vector->size-1);
}

bool vector_set_at(Vector vector, void *data, size_t index)
{
    ERR_EMPTY(vector)
    ERR_BOUNDS(vector, index, vector->size)

    // Destroy old element and replace it
    if (vector->destroy) vector->destroy(vector->array[index]);
    vector->array[index] = data;

    return true;
}

bool vector_prepend(Vector vector, void *data)
{
    // If vector is not empty insert new element before first
    if (vector->size)
        return vector_insert(vector, data, 0);

    // If vector is empty simply set first element
    vector->array[vector->size++] = data;
    return true;
}

bool vector_append(Vector vector, void *data)
{
    // Grow vector if necessary
    if (!vector_grow(vector)) return false;

    // Add element at the end
    vector->array[vector->size++] = data;
    return true;
}

bool vector_insert(Vector vector, void *data, size_t index)
{
    ERR_BOUNDS(vector, index, vector->size)

    // Grow vector if necessary
    if (!vector_grow(vector)) return false;

    // Shift elements and insert new element
    memcpy(vector->array + index + 1, vector->array + index, (vector->size++ - index) * sizeof(void *));
    vector->array[index] = data;

    return true;
}

bool vector_remove_first(Vector vector)
{
    return vector_remove_at(vector, 0);
}

bool vector_remove_last(Vector vector)
{
    return vector_remove_at(vector, vector->size-1);
}

bool vector_remove_at(Vector vector, size_t index)
{
    // Pop data at index
    void *data = vector_pop_at(vector, index);

    // If pop returns NULL, an error occured, flag is already set
    if (!data) return false;

    // Destroy element
    if (vector->destroy) vector->destroy(data);

    return true;
}

bool vector_remove_all(Vector vector, void *data, cmpFunc cmp)
{
    ERR_FUNC(vector, cmp)

    // Number of removed elements
    size_t removed = 0;

    // Traverse elements in vector
    for (size_t i = 0; i < vector->size; i++)
    {
        // If current element is not equal to data
        if (cmp(data, vector->array[i]))
        {
            // Shift it n places where n is the number of removed elements
            vector->array[i - removed] = vector->array[i];
            continue;
        }

        // If current element is equal to data destroy it and increase counter
        if (vector->destroy) vector->destroy(vector->array[i]);
        removed++;
    }

    // Decrease size and shrink if necessary
    vector->size -= removed;

    if (!vector_shrink(vector))
    {
        vector->flag = ALLOC;
        return false;
    }

    return true;
}

void *vector_pop_first(Vector vector)
{
    return vector_pop_at(vector, 0);
}

void *vector_pop_last(Vector vector)
{
    return vector_pop_at(vector, vector->size-1);
}

void *vector_pop_at(Vector vector, size_t index)
{
    ERR_BOUNDS(vector, index, vector->size)

    // Save data at index and shift elements from the right to that position
    void *data = vector->array[index];
    memcpy(vector->array + index, vector->array + index + 1, (--vector->size - index) * sizeof(void *));

    // Shrink vector if necessary
    if (!vector_shrink(vector)) vector->flag = ALLOC;

    return data;
}

size_t vector_index(Vector vector, void *data, cmpFunc cmp)
{
    // Cmp function required
    if (!cmp)
    {
        vector->flag = FUNC;
        return __SIZE_MAX__;
    }

    // Traverse array and return index when found
    for (size_t i = 0; i < vector->size; i++)
        if (!cmp(data, vector->array[i]))
            return i;

    // If element is not found set flag
    vector->flag = ARG;
    return __SIZE_MAX__;
}

size_t vector_count(Vector vector, void *data, cmpFunc cmp)
{
    // Cmp function required
    if (!cmp)
    {
        vector->flag = FUNC; 
        return __SIZE_MAX__;
    }

    // Traverse array and increase count when element is found
    size_t count = 0;
    for (size_t i = 0; i < vector->size; i++)
        if (!cmp(data, vector->array[i]))
            count++;
    
    return count;
}

bool vector_contains(Vector vector, void *data, cmpFunc cmp)
{
    ERR_FUNC(vector, cmp)

    // Traverse array and return true if element is found
    for (size_t i = 0; i < vector->size; i++)
        if (!cmp(data, vector->array[i]))
            return true;
    
    return false;
}

void **vector_array(Vector vector, size_t start, size_t end, copyFunc copy)
{
    if (end <= start)
    {
        vector->flag = ARG;
        return NULL;
    }

    ERR_BOUNDS(vector, end, vector->size + 1);

    // Allocate memory for array
    void **array = malloc((end - start) * sizeof(void *));
    ERR_ALLOC(vector, array)

    // Copy each element of old array
    if (copy)
        for (size_t i = start; i < end; i++)
            array[i] = copy(vector->array[i]);
    else
        for (size_t i = start; i < end; i++)
            array[i] = vector->array[i];
    
    return array;
}

void vector_reverse(Vector vector)
{
    size_t last = vector->size-1;      // Last index of array
    size_t middle = vector->size / 2;  // Middle index of array

    // For every index up to the middle swap its element with the element
    // that has the same index when the array is traversed backwards
    for(size_t i = 0; i < middle; i++)
        swap_elements(vector->array + last - i, vector->array + i);
}

bool vector_trim(Vector vector, size_t start, size_t end)
{
    if (end <= start)
    {
        vector->flag = ARG;
        return NULL;
    }

    ERR_BOUNDS(vector, end, vector->size);

    // Allocate memory for new array
    void *new_array = malloc(vector->size * sizeof(void *));
    ERR_ALLOC(vector, new_array)

    // Destroy elements before start and from end onwards
    if (vector->destroy)
    {
        for (size_t i = 0; i < start; i++)
            vector->destroy(vector->array[i]);
    
        for (size_t i = end; i < vector->size; i++)
            vector->destroy(vector->array[i]);
    }

    // Set new size and change min capacity if necessary
    vector->size = vector->capacity = end - start;

    if (vector->size < vector->min_capacity)
        vector->min_capacity = vector->size;

    // Copy elements to new array
    memcpy(new_array, vector->array + start, vector->size * sizeof(void *));

    // Free old array and replace it
    free(vector->array);
    vector->array = new_array;

    return true;
}


/////////////////////////////// HELPER FUNCTIONS FOR QUICKSORT //////////////////////////////////

// Perform partition to array and return pivot's index
static size_t partition(void **array, size_t low, size_t high, cmpFunc cmp)
{
    void *pivot = array[high]; // Last element is the pivot
    size_t i = low;            // Index where elements smaller than pivot are placed
                               // Starts at the start of the array

    // Traverse every element of the array
    for (size_t j = low; j < high; j++)
    {
        // If element is smaller than pivot, place it in index i and increase index
        if (cmp(array[j], pivot) < 0)
            swap_elements(array + i++, array + j);
    }

    // Put pivot after elements that are smaller and before elements that are greater
    swap_elements(array + i, array + high);

    // Return pivot's position
    return i;
}

// Sort array by quicksort
static void quicksort(void **array, size_t low, size_t high, cmpFunc cmp)
{
    // Perform partition, pivot index is returned
    size_t pi = partition(array, low, high, cmp);

    // Sort arrays left and right of pivot
    if (pi > low) quicksort(array, low, pi-1, cmp);
    if (pi < high) quicksort(array, pi+1, high, cmp);
}

bool vector_sort(Vector vector, cmpFunc cmp)
{
    ERR_FUNC(vector, cmp)

    if (!vector->size)
        return true;

    quicksort(vector->array, 0, vector->size-1, cmp);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

size_t vector_binary_search(Vector vector, void *data, cmpFunc cmp)
{
    if (!cmp)
    {
        vector->flag = ARG;
        return __SIZE_MAX__;
    }

    if (!vector->size) return __SIZE_MAX__;

    // Start and end of the array currently being searched
    size_t low = 0;
    size_t high = vector->size - 1;

    while(low < high)
    {
        // Find middle index and compare element with data
        size_t middle = low + (high - low) / 2;
        int cmp_factor = cmp(data, vector->array[middle]);

        // If they're the same, return that index
        if (!cmp_factor)
            return middle;

        // If data is smaller, search left subarray
        if (cmp_factor < 0)
            high = middle - 1;
        // If data is bigger, search right subarray
        else
            low = middle + 1;
    }

    // If array has one element, check if it is equal to data
    if (low == high)
        if (!cmp(data, vector->array[low]))
            return low;
    
    return __SIZE_MAX__;
}

bool vector_insert_sorted(Vector vector, void *data, cmpFunc cmp)
{
    ERR_FUNC(vector, cmp)

    // If array doesn't contain other elements, simply insert element
    if (!vector->size)
        return vector_append(vector, data);

    // Perform binary search until array that is being searched has size 1
    size_t low = 0;
    size_t high = vector->size - 1;

    while(low < high)
    {
        size_t middle = low + (high - low) / 2;
        int cmp_factor = cmp(data, vector->array[middle]);

        if (!cmp_factor)
            return vector_insert(vector, data, middle);

        if (cmp_factor < 0)
            high = middle ? middle - 1 : middle;
        else
            low = middle + 1;
    }
    
    // Compare data with element in that array
    int cmp_factor = cmp(data, vector->array[low]);

    // If it's bigger add it after element
    if (cmp_factor > 0)
    {
        if (low == vector->size - 1)
            return vector_append(vector, data);
        else
            return vector_insert(vector, data, low+1);
    }
    // If it's smaller add it before
    else return vector_insert(vector, data, low);


}

Vector vector_copy(Vector vector, copyFunc copy)
{
    // Initialize new vector
    Vector vector2 = malloc(sizeof(struct vector));
    ERR_ALLOC(vector, vector2)

    vector2->size = vector->size;
    vector2->capacity = vector->capacity;

    // Allocate memory for new vector's array
    vector2->array = calloc(vector->capacity, sizeof(void *));
    
    if (!vector2->array)
    {
        vector->flag = ALLOC;
        free(vector2); return NULL;
    }

    // If copy function is given, do a deep copy
    if (copy)
    {
        for (size_t i = 0; i < vector->size; i++)
        {
            vector2->array[i] = copy(vector->array[i]);

            // Failed to allocate memory
            if (!vector2->array[i])
            {
                if (vector->destroy)
                {
                    for (size_t j = 0; j < i; j++)
                        vector->destroy(vector2->array[j]);
                }
                
                free(vector2->array); free(vector2);
                vector->flag = ALLOC; return NULL;
            }
        }
    }
    // Else, do a shallow copy
    else memcpy(vector2->array, vector->array, vector->size * sizeof(void *));

    vector2->min_capacity = vector->min_capacity;
    vector2->exp_factor = vector->exp_factor;
    vector2->destroy = vector->destroy;
    vector2->flag = OK;

    return vector2;
}

bool vector_clear(Vector vector)
{
    // Destroy each element
    if (vector->destroy)
        for (size_t i = 0; i < vector->size; i++)
            vector->destroy(vector->array[i]);

    free(vector->array);

    // Allocate new array at min capacity
    vector->array = calloc(vector->min_capacity, sizeof(void *));
    ERR_ALLOC(vector, vector->array)

    vector->size = 0;
    vector->capacity = vector->min_capacity;
    return true;
}

void vector_destroy(Vector vector)
{
    if (vector->destroy)
        for (size_t i = 0; i < vector->size; i++)
            vector->destroy(vector->array[i]);

    free(vector->array);
    free(vector);
}

size_t vector_capacity(Vector vector)
{
    return vector->capacity;
}

size_t vector_min_capacity(Vector vector)
{
    return vector->min_capacity;
}

double vector_exp_factor(Vector vector)
{
    return vector->exp_factor;
}

void vector_set_destroy(Vector vector, destroyFunc destroy)
{
    vector->destroy = destroy;
}

bool vector_set_min_capacity(Vector vector, size_t min_capacity)
{
    if (!min_capacity)
    {
        vector->flag = ARG;
        return false;
    }

    vector->min_capacity = min_capacity;
    return true;
}

bool vector_set_exp_factor(Vector vector, double exp_factor)
{
    if (exp_factor <= 1)
    {
        vector->flag = ARG;
        return false;
    }

    vector->exp_factor = exp_factor;
    return true;
}

int vector_flag(Vector vector)
{
    return vector->flag;
}