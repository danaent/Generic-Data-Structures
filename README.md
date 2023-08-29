# Generic Data Structures in C

A library that contains generic implementations of commonly used data structures. The data structures currently available are:

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

The library file can be created by running ```make lib``` inside the [lib](https://github.com/danaent/Generic-Data-Structures/blob/main/example) directory. Make sure to include that file during compilation.
```bash
$ gcc -o prog main.c -L. lib/libgds.a
```

### Initialization and de-allocation
Every data structure needs to be initialized before any operations and destroyed in the end so that all memory used by the structure or its elements is freed. These functions are included in every data structure:
```c
Stack my_stack = stack_init(free);

// ... some stack operations
// the stack is no longer needed

stack_destroy(my_stack);
```

### Generic functions
Often, an operation will require a user-made function that is specific to the elements' data type to work. Sometimes these functions are essential to the data structure and need to be inputed during initialization (when the ```init``` function is called) so that they can be used for multiple operations. Other times, they are only required for a specific function and need to be inputed when that function is called. They can be one of the following:

| Function   | Description |
|:----------:|:------------|
|**[destroy](https://github.com/danaent/Generic-Data-Structures/blob/main/include/func.h#L9)**| Free all memory allocated for element. If the argument is ``````NULL``````, it should have no effect.<br><br>This function is required to initialize all data structures and is used by functions that delete elements. When set to ```NULL```, memory is not de-allocated for the elements held by the pointers stored in the data structure.|
|**[cmp](https://github.com/danaent/Generic-Data-Structures/blob/main/include/func.h#L22)**| Return an integer that determines the order of the two elements. If p1 and p2 are passed into the function, its return values should be:<br><br>```< 0``` if p1 precedes p2, or has a higher priority<br>```= 0``` if p1 has the same priority as p2<br>```> 0``` if p1 follows p2, or has a lower priority<br><br>This function is required for data structures where each element is associated with a priority, such as the priority queue or the B-tree. It cannot be ```NULL```.|
|**[copy](https://github.com/danaent/Generic-Data-Structures/blob/main/include/func.h#L30)**| Return a copy of element. If the function returns ```NULL```, allocation failure is assumed.<br><br>This function is required for functions that return a copy of a data structure. If set to ```NULL```, the pointers of the structure are copied, but not the elements they hold.|
|**[hash](https://github.com/danaent/Generic-Data-Structures/blob/main/include/func.h#L35)**|Hash element to an unsigned integer.<br><br>This function is required for the hash table. It cannot be ```NULL```.|


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