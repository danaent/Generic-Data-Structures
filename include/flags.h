enum str_err_flags 
{
    OK      = 0, // No errors
    ALLOC   = 1, // Failure to allocate memory
    EMPTY   = 2, // Attempt to access elements of an empty structure
    BOUNDS  = 3, // Index out of bounds
    ARG     = 4, // Invalid argument
    FUNC    = 5  // Missing necessary function for operation
};


// MACROS FOR QUICK ERROR CHECKING

#define ERR_ALLOC(adt, ptr) \
if (!ptr) \
{ \
    adt->flag = ALLOC; \
    return 0; \
}

#define ERR_EMPTY(adt) \
if (!adt->size) \
{ \
    adt->flag = EMPTY; \
    return 0; \
}

#define ERR_FUNC(adt, func) \
if (!func) \
{ \
    adt->flag = FUNC; \
    return 0; \
}

#define ERR_BOUNDS(adt, i, max) \
if (i >= max) \
{ \
    adt->flag = BOUNDS; \
    return 0; \
}
