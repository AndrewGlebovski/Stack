#include <stdio.h>
#include "stack.hpp"


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


const float STACK_FACTOR = 2.0;


ErrorBits stack_resize(Stack *stack, StackSize capacity);


void print_binary(ErrorBits n);


void set_hash(Stack *stack);


ErrorBits check_struct_hash(Stack *stack);


HashType gnu_hash(void *ptr, size_t size);




ErrorBits stack_constructor(Stack *stack, StackSize capacity) {
    char *true_pointer = (char *) calloc(capacity * sizeof(Object) + 2 * sizeof(CanaryType), 1);
    CHECK(true_pointer, return ERROR_BIT_FLAGS::ALLOCATE_FAIL);

    *(CanaryType *)(true_pointer) = CANARY_VALUE;
    *(CanaryType *)(true_pointer + sizeof(CanaryType) + capacity * sizeof(Object)) = CANARY_VALUE;

    stack -> data = (Object *)(true_pointer + sizeof(CanaryType));

    for(StackSize i = 0; i < capacity ; i++)
        (stack -> data)[i] = POISON_VALUE;
    
    stack -> capacity = capacity;
    stack -> size = 0;

    set_hash(stack);

    stack -> canary_begin = CANARY_VALUE;
    stack -> canary_end = CANARY_VALUE;

    return ERROR_BIT_FLAGS::STACK_OK;
}


ErrorBits stack_resize(Stack *stack, StackSize capacity) {
    CHECK(stack, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);
    CHECK(capacity >= 10, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);

    ErrorBits error = stack_check(stack);
    CHECK(!error, return error);

    char *true_pointer = ((char *)(stack -> data)) - sizeof(CanaryType);
    true_pointer = (char *) realloc(true_pointer, capacity * sizeof(Object) + 2 * sizeof(CanaryType));
    CHECK(true_pointer, return ERROR_BIT_FLAGS::ALLOCATE_FAIL);

    *(CanaryType *)(true_pointer + sizeof(CanaryType) + capacity * sizeof(Object)) = CANARY_VALUE;

    stack -> data = (Object *)(true_pointer + sizeof(CanaryType));

    for(StackSize i = stack -> capacity; i < capacity ; i++)
        (stack -> data)[i] = POISON_VALUE;
    
    stack -> capacity = capacity;

    set_hash(stack);

    return error;
}


ErrorBits stack_push(Stack *stack, Object object) {
    CHECK(stack, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);

    ErrorBits error = stack_check(stack);
    CHECK(!error, return error);

    if ((stack -> size) + 1 > stack -> capacity)
        error = stack_resize(stack, (StackSize) ((float)(stack -> capacity) * STACK_FACTOR));

    CHECK(!error, return error);

    (stack -> data)[(stack -> size)++] = object;

    set_hash(stack);

    return error;
}


ErrorBits stack_pop(Stack *stack, Object *object) {
    CHECK(stack, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);
    CHECK(object, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);

    ErrorBits error = stack_check(stack);
    CHECK(!error, return error);

    CHECK(stack -> size, return ERROR_BIT_FLAGS::EMPTY_STACK);

    *object = (stack -> data)[--(stack -> size)];
    (stack -> data)[(stack -> size)] = POISON_VALUE;

    set_hash(stack);

    if (((stack -> size) < ((stack -> capacity) / (StackSize)(2 * STACK_FACTOR))) && stack -> capacity > 10)
        error = stack_resize(stack, (stack -> capacity) / (StackSize)(STACK_FACTOR));

    return error;
}


ErrorBits stack_destructor(Stack *stack) {
    ErrorBits error = stack_check(stack);
    CHECK(!error, return error);

    free(stack -> data);
    stack -> data = NULL;
    
    stack -> capacity = 0;
    stack -> size = 0;

    return ERROR_BIT_FLAGS::STACK_OK;
}


ErrorBits stack_check(Stack *stack) {
    ErrorBits error = ERROR_BIT_FLAGS::STACK_OK;

    CHECK(stack, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);

    CHECK(stack -> canary_begin == CANARY_VALUE && stack -> canary_end == CANARY_VALUE, return ERROR_BIT_FLAGS::STRUCT_CANARY);

    CHECK(!check_struct_hash(stack), return ERROR_BIT_FLAGS::STRUCT_HASH_FAIL);

    char *true_pointer = ((char *)(stack -> data)) - sizeof(CanaryType); // pointer to the real buffer start

    CHECK(*(CanaryType *)(true_pointer) == CANARY_VALUE,                                                           return ERROR_BIT_FLAGS::BUFFER_CANARY);
    CHECK(*(CanaryType *)(true_pointer + sizeof(CanaryType) + stack -> capacity * sizeof(Object)) == CANARY_VALUE, return ERROR_BIT_FLAGS::BUFFER_CANARY);

    CHECK(stack -> capacity >= 0 && stack -> capacity <= MAX_CAPACITY_VALUE, error += ERROR_BIT_FLAGS::INVALID_CAPACITY);

    CHECK(stack -> size >= 0 && stack -> size <= stack -> capacity, error += ERROR_BIT_FLAGS::INVALID_SIZE);

    CHECK(stack -> data, error += ERROR_BIT_FLAGS::NULL_DATA; return error);

    CHECK(gnu_hash(stack -> data, stack -> size * sizeof(Object)) == stack -> buffer_hash, error += ERROR_BIT_FLAGS::BUFFER_HASH_FAIL);

    if (HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_SIZE) || HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_CAPACITY) || HAS_ERROR(error, ERROR_BIT_FLAGS::STRUCT_HASH_FAIL))
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

    printf("Capacity: %llu\nSize: %llu\nBuffer hash: %llu\nStruct hash: %llu\nData[%p]:\n", 
            stack -> capacity, stack -> size, stack -> buffer_hash, stack -> struct_hash, stack -> data);
    
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::NULL_DATA) || HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_CAPACITY) 
            || HAS_ERROR(error, ERROR_BIT_FLAGS::STRUCT_HASH_FAIL) || HAS_ERROR(error, ERROR_BIT_FLAGS::STRUCT_CANARY)) 
        return;

    for(StackSize i = 0; i < stack -> capacity; i++) {
        printf("    [%lld]", i); // object index

        printf("%d", (stack -> data)[i]); // print value function (possible macros)

        if ((stack -> data)[i] == POISON_VALUE) printf("(POISON VALUE)"); // poison value warning
        
        putchar('\n'); // new line
    }

    putchar('\n');
}


void print_errors(ErrorBits error) {
    if (error == ERROR_BIT_FLAGS::STACK_OK) {
        printf("Ok\n");
        return;
    }
    else {
        printf("Error ");
        print_binary(error);
        putchar('\n');
    }

    if (HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_SIZE))     printf("Invalid size\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_CAPACITY)) printf("Invalid capcity\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::NULL_DATA))        printf("Stack has NULL data\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::UNEXP_NORMAL_VAL)) printf("Unexpected normal value\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::UNEXP_POISON_VAL)) printf("Unexpected poison value\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::STRUCT_CANARY))    printf("Wrong struct canary\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::BUFFER_CANARY))    printf("Wrong buffer canary\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::EMPTY_STACK))      printf("Pop empty stack\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_ARGUMENT)) printf("Invalid argument given to the function\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::ALLOCATE_FAIL))    printf("Failed to allocate memory\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::BUFFER_HASH_FAIL)) printf("Wrong buffer hash\n");
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::STRUCT_HASH_FAIL)) printf("Wrong struct hash\n");
}


void print_binary(ErrorBits n) {
    int k = 1 << 15;
    while(k > 0) {
        putchar(((n & k) > 0) + '0');
        k = k >> 1;
    }
}


ErrorBits check_struct_hash(Stack *stack) {
    CHECK(stack, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);

    ErrorBits error = ERROR_BIT_FLAGS::STACK_OK;

    HashType h1 = stack -> struct_hash, h2 = stack -> buffer_hash;

    stack -> struct_hash = 0;
    stack -> buffer_hash = 0;

    CHECK(gnu_hash(stack, sizeof(Stack)) == h1, error += ERROR_BIT_FLAGS::STRUCT_HASH_FAIL);

    stack -> struct_hash = h1;
    stack -> buffer_hash = h2;

    return error;
}


void set_hash(Stack *stack) {
    CHECK(stack, return);

    stack -> struct_hash = 0;
    stack -> buffer_hash = 0;

    stack -> struct_hash = gnu_hash(stack, sizeof(Stack));
    stack -> buffer_hash = gnu_hash(stack -> data, stack -> size * sizeof(Object));
}


HashType gnu_hash(void *ptr, size_t size) {
    HashType hash = 5381;

    for(size_t i = 0; i < size; i++)
        hash = hash * 33 + ((char *)(ptr))[i];

    return hash;
}
