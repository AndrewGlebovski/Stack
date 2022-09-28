#define POISON_VALUE 0xC0FFEE
#define MAX_CAPACITY_VALUE 100000
#define OBJECT_TO_STR "%i"

#define CANARY_PROTECT 1
#define HASH_PROTECT 2


#ifndef PROTECT_LEVEL
    #define PROTECT_LEVEL (CANARY_PROTECT + HASH_PROTECT)
#endif


#if (PROTECT_LEVEL & CANARY_PROTECT)
    #define ON_CANARY_PROTECT(...) __VA_ARGS__
#else
    #define ON_CANARY_PROTECT(...) 
#endif


#if (PROTECT_LEVEL & HASH_PROTECT)
    #define ON_HASH_PROTECT(...) __VA_ARGS__
#else
    #define ON_HASH_PROTECT(...) 
#endif


typedef int Object;
typedef long long StackSize;
typedef unsigned long long ErrorBits;
typedef unsigned long long CanaryType;
typedef unsigned long long HashType;


typedef struct {
    ON_CANARY_PROTECT(CanaryType canary_begin = 0;)

    Object *data = NULL;
    StackSize size = 0;
    StackSize capacity = 0;

    ON_HASH_PROTECT(HashType struct_hash = 0;)
    ON_HASH_PROTECT(HashType buffer_hash = 0;)

    ON_CANARY_PROTECT(CanaryType canary_end = 0;)
} Stack;


enum ERROR_BIT_FLAGS {
    STACK_OK         =    0, ///< Stack is ok
    NULL_DATA        =    1, ///< Stack data points to NULL
    INVALID_SIZE     =    2, ///< Size is larger than capacity or negative
    INVALID_CAPACITY =    4, ///< Capacity is larger than type's maximum or negative
    UNEXP_POISON_VAL =    8, ///< Unexpected poison value before size
    UNEXP_NORMAL_VAL =   16, ///< Unexpected normal value after size
    INVALID_ARGUMENT =   32, ///< Invalid argument given to the function
    EMPTY_STACK      =   64, ///< No elements to pop
    ALLOCATE_FAIL    =  128, ///< Memory allocate return NULL
    STRUCT_CANARY    =  256, ///< Stack canary value was overwritten
    BUFFER_CANARY    =  512, ///< Buffer canary value was overwritten
    BUFFER_HASH_FAIL = 1024, ///< Wrong buffer hash sum
    STRUCT_HASH_FAIL = 2048, ///< Wrong stack hash sum
};


ErrorBits stack_constructor(Stack *stack, StackSize capacity);


ErrorBits stack_push(Stack *stack, Object object);


ErrorBits stack_pop(Stack *stack, Object *object);


ErrorBits stack_destructor(Stack *stack);


ErrorBits stack_check(Stack *stack);


void stack_dump(Stack *stack, ErrorBits error);


void print_errors(ErrorBits error);
