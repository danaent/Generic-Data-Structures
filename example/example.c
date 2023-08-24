#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/list.h" // Include the header files of the data structures you want to use

// Comparator function for strings
int str_compare(const void *str1, const void *str2)
{
    return strcmp(str1, str2);
}

int main(void)
{
    // All data structures need to be initialized before use. We initialize a list
    // that will handle strings. We don't need memory to be de-allocated for those
    // strings, so we set the destroy function to NULL.
    List str_list = list_init(NULL);

    // Add some strings to the list
    list_append(str_list, "Hello");
    list_append(str_list, "world");
    list_append(str_list, "of");
    list_append(str_list, "generic");
    list_append(str_list, "data");
    list_append(str_list, "structures");

    // Get the size of the list and print it
    size_t size = list_size(str_list);
    printf("Size of list is %d\n", (int) size);

    // Retrieve the first string and print it
    char *str = list_get_first(str_list);
    printf("%s!\n", str);

    // This function requires a compare function. We use the one we made.
    list_sort(str_list, str_compare);

    // Now get every string from the list and print it
    while (!list_empty(str_list))
    {
        str = list_pop_first(str_list);
        printf("%s ", str);
    }
    printf("\n");

    // Once we're done, memory used by the data structure needs to be freed
    list_destroy(str_list);
}