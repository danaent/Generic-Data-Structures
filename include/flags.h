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

#define ERR_ALLOC(str, ptr) \
if (!ptr) \
{ \
    str->flag = ALLOC; \
    return 0; \
}

#define ERR_EMPTY(str) \
if (!str->size) \
{ \
    str->flag = EMPTY; \
    return 0; \
}

#define ERR_FUNC(str, func) \
if (!func) \
{ \
    str->flag = FUNC; \
    return 0; \
}

#define ERR_BOUNDS(str, i, max) \
if (i >= max) \
{ \
    str->flag = BOUNDS; \
    return 0; \
}
