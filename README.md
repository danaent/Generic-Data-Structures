# Generic Data Structures in C

This is a library that contains generic implementations of commonly used data structures. The data structures currently available are:

- Stack
- Queue
- Vector
- Doubly-linked list
- Priority Queue
- Hash table
- B-Tree

## How to Use

### Including in your project
To use the library, you'll need to include the header file into your project:
```c
#include "include/libgds.h"
```

If you don't wish to include every data structure, you can include only the header files of the data structures you need:
```c
#include "include/pq.h"
#include "include/hashtable.h"
```

During compilation, make sure to include the library file:
```bash
$ gcc -o prog main.c -L. lib/libgds.a
```

### Initialization and de-allocation
Every data structure needs to be initialized before any operations and destroyed in the end so that all memory used by the strucure or its elements is freed. These functions are included in every data structure:
```c
Stack my_stack = stack_init(free);

// ... some stack operations
// the stack is no longer needed

stack_destroy(my_stack);
```

### Generic functions
Often, an operation will require a user-made function that is specific to the elements' data type to work. This can be one of the following:
- **[destroy function](https://github.com/danaent/Generic-Data-Structures/blob/main/include/func.h#L9):** Free all memory allocated for element in structure. Required to initialize all data structures.
- **[compare function](https://github.com/danaent/Generic-Data-Structures/blob/main/include/func.h#L22):** Compare two elements and return an integer that determines their order. Required to initialize some data structures and for some function calls.
- **[copy function](https://github.com/danaent/Generic-Data-Structures/blob/main/include/func.h#L30):** Return a copy of the element. Required for functions that copy data structures.
- **[hash function](https://github.com/danaent/Generic-Data-Structures/blob/main/include/func.h#L35):** Hash element to an unsigned integer. Required for the hash table.

Full explanation on how each of these functions should work is [here](https://github.com/danaent/Generic-Data-Structures/blob/main/include/func.h).


### Error flags
Each data structure holds a flag with an integer showing if errors have occured during its usage. Different integers indicate different errors, zero indicating no errors. All flag numbers can be found [here](https://github.com/danaent/Generic-Data-Structures/blob/main/include/flags.h). You can always check a structure's flag to see if any errors have occured:
```c
int a = 5;

// Push element to stack
stack_push(my_stack, &a);

// Check if error has occured
assert(stack_flag(my_stack) == 0);
```

## Example
A detailed example of a list's usage can be found in the [example](https://github.com/danaent/Generic-Data-Structures/blob/main/example) directory. For more infomation on how each data structure works, check out their individual header files.