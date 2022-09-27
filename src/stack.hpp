#include <stdlib.h>


/**
 * \brief This assert throw errors to stderr
 * \param [in] condition Condition to check
 * \param [in] error This error code will be printed if condition fails
 * \param [in] msg This message will be printed if condition fails
 * \param [in] action This code will be executed if condition fails
*/
#define ASSERT(condition, error, msg, action) \
do { \
    if (!(condition)) { \
        fprintf(stderr, "%s in function %s in line %d\n[%.3d] %s\n\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, error, msg); \
        action; \
    } \
} while(0)


/**
 * \brief Do some action in case of error
 * \param [in] condition Condition to check
 * \param [in] action This code will be executed if condition fails
*/
#define CHECK(condition, action) \
do { \
    if (!(condition)) { \
        action; \
    } \
} while(0)


/**
 * \brief Prints stack
 * \param [in] stack Stack to print
*/
#define STACK_DUMP(stack) \
do { \
    if (!(condition)) { \
        fprintf(stderr, "%s at %s(%d)", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
        stack_dump(stack); \
    } \
} while(0)


/**
 * \brief Prints stack
 * \param [in] stack Stack to print
*/
#define STACK_CHECK(stack) \
do { \
    if (!(condition)) { \
        fprintf(stderr, "%s at %s(%d)", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
        stack_dump(stack); \
    } \
} while(0)


#define HAS_ERROR(bitflag, error) (bitflag & error)


#define POISON_VALUE 0xC0FFEE
#define MAX_CAPACITY_VALUE 100000
#define OBJECT_TO_STR "%i"


const float STACK_FACTOR = 2.0;


typedef int Object;
typedef long long StackSize;
typedef unsigned int ErrorBits;


typedef struct Stack
{
    Object *data = NULL;
    StackSize size = 0;
    StackSize capacity = 0;
} Stack;


enum ERROR_BIT_FLAGS {
    STACK_OK         =   0, ///< Stack is ok
    NULL_DATA        =   1, ///< Stack data points to NULL
    INVALID_SIZE     =   2, ///< Size is larger than capacity or negative
    INVALID_CAPACITY =   4, ///< Capacity is larger than type's maximum or negative
    UNEXP_POISON_VAL =   8, ///< Unexpected poison value before size
    UNEXP_NORMAL_VAL =  16, ///< Unexpected normal value after size
    INVALID_ARGUMENT =  32, ///< Invalid argument given to the function
    EMPTY_STACK      =  64, ///< No elements to pop
    ALLOCATE_FAIL    = 128, ///< Memory allocate return NULL
};


ErrorBits stack_constructor(Stack *stack, StackSize capacity);


ErrorBits stack_resize(Stack *stack, StackSize capacity);


ErrorBits stack_push(Stack *stack, Object object);


ErrorBits stack_pop(Stack *stack, Object *object);


ErrorBits stack_destructor(Stack *stack);


ErrorBits stack_check(Stack *stack);


void stack_dump(Stack *stack, ErrorBits error);


void print_errors(ErrorBits error);


void print_binary(ErrorBits n);




ErrorBits stack_constructor(Stack *stack, StackSize capacity) {
    stack -> data = (Object *) calloc(capacity, sizeof(Object));
    CHECK(stack -> data, return ERROR_BIT_FLAGS::ALLOCATE_FAIL);

    for(StackSize i = 0; i < capacity ; i++)
        (stack -> data)[i] = POISON_VALUE;
    
    stack -> capacity = capacity;
    stack -> size = 0;

    return ERROR_BIT_FLAGS::STACK_OK;
}


ErrorBits stack_resize(Stack *stack, StackSize capacity) {
    ErrorBits error = stack_check(stack);

    if (error) return error;

    stack -> data = (Object *) realloc(stack -> data, capacity * sizeof(Object));
    CHECK(stack -> data, return ERROR_BIT_FLAGS::ALLOCATE_FAIL);

    for(StackSize i = stack -> capacity; i < capacity ; i++)
        (stack -> data)[i] = POISON_VALUE;
    
    stack -> capacity = capacity;

    return error;
}


ErrorBits stack_push(Stack *stack, Object object) {
    ErrorBits error = stack_check(stack);

    if (error) return error;

    if ((stack -> size) + 1 > stack -> capacity)
        error = stack_resize(stack, (StackSize) ((float)(stack -> capacity) * STACK_FACTOR));

    if (error) return error;

    (stack -> data)[(stack -> size)++] = object;

    return error;
}


ErrorBits stack_pop(Stack *stack, Object *object) {
    ErrorBits error = stack_check(stack);

    if (error) return error;
    if (stack -> size == 0) return ERROR_BIT_FLAGS::EMPTY_STACK;

    CHECK(object, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);

    *object = (stack -> data)[--(stack -> size)];
    (stack -> data)[(stack -> size)] = POISON_VALUE;

    if (((stack -> size) < ((stack -> capacity) / (StackSize)(2 * STACK_FACTOR))) && stack -> capacity > 10)
        error = stack_resize(stack, (stack -> capacity) / (StackSize)(STACK_FACTOR));

    return error;
}


ErrorBits stack_destructor(Stack *stack) {
    ErrorBits error = stack_check(stack);

    if (error) return error;

    free(stack -> data);
    stack -> data = NULL;
    
    stack -> capacity = 0;
    stack -> size = 0;

    return ERROR_BIT_FLAGS::STACK_OK;
}


ErrorBits stack_check(Stack *stack) {
    ErrorBits error = ERROR_BIT_FLAGS::STACK_OK;

    CHECK(stack, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);

    CHECK(stack -> capacity >= 0 && stack -> capacity <= MAX_CAPACITY_VALUE, error += ERROR_BIT_FLAGS::INVALID_CAPACITY);

    CHECK(stack -> size >= 0 && stack -> size <= stack -> capacity, error += ERROR_BIT_FLAGS::INVALID_SIZE);

    CHECK(stack -> data, error += ERROR_BIT_FLAGS::NULL_DATA; putchar('t'); return error);

    if (HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_SIZE) || HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_CAPACITY))
        return error;

    for(StackSize i = 0; i < stack -> capacity; i++) {
        if (i < stack -> size)
            CHECK((stack -> data)[i] != POISON_VALUE, error += ERROR_BIT_FLAGS::UNEXP_POISON_VAL; i = stack -> size);
        else
            CHECK((stack -> data)[i] == POISON_VALUE, error += ERROR_BIT_FLAGS::UNEXP_NORMAL_VAL; break);
    }
    
    return error;
}


void stack_dump(Stack *stack, ErrorBits error) {
    CHECK(stack, return);
    
    printf("Stack[%p]:\n", stack);

    print_errors(error);

    printf("Capacity: %lld\nSize: %lld \nData[%p]:\n", stack -> capacity, stack -> size, stack -> data);
    
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::NULL_DATA) || HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_CAPACITY)) return;

    for(StackSize i = 0; i < stack -> capacity; i++) {
        printf("    [%lld]", i); // object index

        printf("%d", (stack -> data)[i]); // print value function (possible macros)

        if ((stack -> data)[i] == POISON_VALUE) printf("(POISON VALUE)"); // poison value warning
        
        putchar('\n'); // new line
    }
}


void print_errors(ErrorBits error) {
    printf("Error bit flag: ");
    print_binary(error);
    putchar('\n');

    if (error == ERROR_BIT_FLAGS::STACK_OK) {
        printf("Stack is ok\n");
        return;
    }

    if (HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_SIZE))     printf("Invalid size\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_CAPACITY)) printf("Invalid capcity\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::NULL_DATA))        printf("Stack has NULL data\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::UNEXP_NORMAL_VAL)) printf("Unexpected normal value\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::UNEXP_POISON_VAL)) printf("Unexpected poison value\n");
}


void print_binary(ErrorBits n) {
    int k = 1 << 7;
    while(k > 0) {
        putchar(((n & k) > 0) + '0');
        k = k >> 1;
    }
}
