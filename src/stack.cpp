#include <stdlib.h>
#include <windows.h>
#include "stack.hpp"
#include "logs.hpp"


const char *ERROR_DESCRIPTION[] = {
    "Invalid data pointer\n",
    "Invalid size\n",
    "Invalid capcity\n",
    "Unexpected poison value\n",
    "Unexpected normal value\n",
    "Invalid argument given to the function\n",
    "Pop empty stack\n",
    "Failed to allocate memory\n",
    "Wrong struct canary\n",
    "Wrong buffer canary\n",
    "Wrong buffer hash\n",
    "Wrong struct hash\n",
    "Invalid read pointer\n",
};


/**
 * \brief Does some action in case of error
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
 * \brief Prints stack's content
 * \param [in] stack Stack to print
 * \param [in] error This error code will printed (see #ERROR_BIT_FLAGS and print_errors())
*/
#define STACK_DUMP(stack, error) \
do { \
    if (get_log_file()) { \
        fprintf(get_log_file(), "%s at %s(%d)\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
        stack_dump(stack, error, get_log_file()); \
    } \
} while(0)


/**
 * \brief If stack is invalid calls stack dump then returns an error code
 * \param [in] stack Stack to check
*/
#define RETURN_ON_ERROR(stack) \
do { \
    ErrorBits error = stack_check(stack); \
    if (error) { \
        STACK_DUMP(stack, error); \
        return error; \
    } \
} while(0)


/// Checks for specific error in error code (see #ERROR_BIT_FLAGS and #ErrorBits type)
#define HAS_ERROR(bitflag, error) (bitflag & error)


/**
 * \brief Resizes stack
 * \param stack This stack will be resized automaticaly
 * \return Error code (see #ERROR_BIT_FLAGS)
*/
static ErrorBits stack_resize(Stack *stack);


/**
 * \brief Recursive function to print each bit of the number
 * \param n This number will be printed
 * \param stream File to print bin code in
*/
static void print_binary(ErrorBits n, FILE *stream);


/**
 * \brief Checks bad read pointer
 * \param ptr Pointer to check
 * \param size Pointer size
 * \return Zero value means error
*/
static int right_pointer(void *ptr, size_t size);

#if (PROTECT_LEVEL & HASH_PROTECT)

/**
 * \brief Recalculates hash sum for current stack
 * \param stack This stack's hash sum will be updated
*/
static void set_hash(Stack *stack);


/**
 * \brief Check hash sum of stack structure
 * \param stack This stack's hash sum will be checked
*/
static ErrorBits check_struct_hash(Stack *stack);


/**
 * \brief Calculates hash sum for object
 * \param ptr Pointer to object
 * \param size Object's size
 * \return Hash sum
*/
static HashType gnu_hash(void *ptr, size_t size);

#endif




ErrorBits stack_constructor(Stack *stack, StackSize capacity) {
    CHECK(right_pointer(stack, sizeof(Stack)), return ERROR_BIT_FLAGS::INVALID_POINTER);
    CHECK(capacity > 0, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);

    #if (PROTECT_LEVEL & CANARY_PROTECT)
        char *true_pointer = (char *) calloc(capacity * sizeof(Object) + 2 * sizeof(CanaryType), 1);
        CHECK(true_pointer, return ERROR_BIT_FLAGS::ALLOCATE_FAIL);

        ON_CANARY_PROTECT(*(CanaryType *)(true_pointer) = (CanaryType)(stack);)
        ON_CANARY_PROTECT(*(CanaryType *)(true_pointer + sizeof(CanaryType) + capacity * sizeof(Object)) = (CanaryType)(stack);)

        stack -> data = (Object *)(true_pointer + sizeof(CanaryType));
    #else
        stack -> data = (Object *)calloc(capacity, sizeof(Object));
        CHECK(stack -> data, return ERROR_BIT_FLAGS::ALLOCATE_FAIL);
    #endif

    for(StackSize i = 0; i < capacity ; i++)
        (stack -> data)[i] = POISON_VALUE;
    
    stack -> capacity = capacity;
    stack -> size = 0;

    ON_CANARY_PROTECT(stack -> canary_begin = (CanaryType)(stack);)
    ON_CANARY_PROTECT(stack -> canary_end = (CanaryType)(stack);)

    ON_HASH_PROTECT(set_hash(stack);)

    RETURN_ON_ERROR(stack);

    return ERROR_BIT_FLAGS::STACK_OK;
}


static ErrorBits stack_resize(Stack *stack) {
    RETURN_ON_ERROR(stack);

    if (4 * stack -> size < stack -> capacity)
        stack -> capacity /= 2;
    
    else if (stack -> size == stack -> capacity)
        stack -> capacity *= 2;

    else 
        return ERROR_BIT_FLAGS::STACK_OK;

    #if (PROTECT_LEVEL & CANARY_PROTECT)
        char *true_pointer = ((char *)(stack -> data)) - sizeof(CanaryType);
        true_pointer = (char *) realloc(true_pointer, stack -> capacity * sizeof(Object) + 2 * sizeof(CanaryType));
        CHECK(true_pointer, return ERROR_BIT_FLAGS::ALLOCATE_FAIL);

        *(CanaryType *)(true_pointer + sizeof(CanaryType) + stack -> capacity * sizeof(Object)) = (CanaryType)(stack);

        stack -> data = (Object *)(true_pointer + sizeof(CanaryType));
    #else
        stack -> data = (Object *) realloc(stack -> data, capacity * sizeof(Object));
        CHECK(stack -> data, return ERROR_BIT_FLAGS::ALLOCATE_FAIL);
    #endif

    for(StackSize i = stack -> size; i < stack -> capacity ; i++)
        (stack -> data)[i] = POISON_VALUE;

    ON_HASH_PROTECT(set_hash(stack);)

    RETURN_ON_ERROR(stack);

    return ERROR_BIT_FLAGS::STACK_OK;
}


ErrorBits stack_push(Stack *stack, Object object) {
    RETURN_ON_ERROR(stack);

    (stack -> data)[(stack -> size)++] = object;

    ON_HASH_PROTECT(set_hash(stack);)

    RETURN_ON_ERROR(stack);

    return stack_resize(stack);
}


ErrorBits stack_pop(Stack *stack, Object *object) {
    CHECK(object, return ERROR_BIT_FLAGS::INVALID_ARGUMENT);

    RETURN_ON_ERROR(stack);

    CHECK(stack -> size, return ERROR_BIT_FLAGS::EMPTY_STACK);

    *object = (stack -> data)[--(stack -> size)];
    (stack -> data)[(stack -> size)] = POISON_VALUE;

    ON_HASH_PROTECT(set_hash(stack);)

    RETURN_ON_ERROR(stack);

    return stack_resize(stack);
}


ErrorBits stack_destructor(Stack *stack) {
    RETURN_ON_ERROR(stack);

    #if (PROTECT_LEVEL & CANARY_PROTECT)
        free((char *)(stack -> data) - sizeof(CanaryType));
    #else
        free((char *)(stack -> data));
    #endif

    stack -> data = NULL;
    
    stack -> capacity = 0;
    stack -> size = 0;

    ON_HASH_PROTECT(set_hash(stack);)

    return ERROR_BIT_FLAGS::STACK_OK;
}


ErrorBits stack_check(Stack *stack) {
    ErrorBits error = ERROR_BIT_FLAGS::STACK_OK;

    CHECK(right_pointer(stack, sizeof(Stack)), return ERROR_BIT_FLAGS::INVALID_POINTER);

    ON_CANARY_PROTECT(CHECK(stack -> canary_begin == (CanaryType)(stack) && stack -> canary_end == (CanaryType)(stack), return ERROR_BIT_FLAGS::STRUCT_CANARY);)

    ON_HASH_PROTECT(CHECK(!check_struct_hash(stack), return ERROR_BIT_FLAGS::STRUCT_HASH_FAIL);)

    CHECK(right_pointer(stack -> data, stack -> capacity * sizeof(Object)), error += ERROR_BIT_FLAGS::INVALID_DATA; return error);

    ON_CANARY_PROTECT(char *true_pointer = ((char *)(stack -> data)) - sizeof(CanaryType);) // pointer to the real buffer start

    ON_CANARY_PROTECT(CHECK(*(CanaryType *)(true_pointer) == (CanaryType)(stack),                                                           return ERROR_BIT_FLAGS::BUFFER_CANARY);)
    ON_CANARY_PROTECT(CHECK(*(CanaryType *)(true_pointer + sizeof(CanaryType) + stack -> capacity * sizeof(Object)) == (CanaryType)(stack), return ERROR_BIT_FLAGS::BUFFER_CANARY);)

    CHECK(stack -> capacity >= 0 && stack -> capacity <= MAX_CAPACITY_VALUE, error += ERROR_BIT_FLAGS::INVALID_CAPACITY);

    CHECK(stack -> size >= 0 && stack -> size <= stack -> capacity, error += ERROR_BIT_FLAGS::INVALID_SIZE);

    ON_HASH_PROTECT(CHECK(gnu_hash(stack -> data, stack -> size * sizeof(Object)) == stack -> buffer_hash, error += ERROR_BIT_FLAGS::BUFFER_HASH_FAIL);)

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


void stack_dump(Stack *stack, ErrorBits error, FILE *stream) {
    CHECK(right_pointer(stack, sizeof(stack)), return);
    
    fprintf(stream, "\tStack[%p]:\n", stack);

    print_errors(error, stream);

    fprintf(stream, "\tCapacity: %llu\n\tSize: %llu\n", stack -> capacity, stack -> size);

    ON_HASH_PROTECT(fprintf(stream, "\tBuffer hash: %0llx\n\tStruct hash: %0llx\n", stack -> buffer_hash, stack -> struct_hash);)

    fprintf(stream, "\tData[%p]", stack -> data);
    
    if (HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_DATA) || HAS_ERROR(error, ERROR_BIT_FLAGS::INVALID_CAPACITY) 
            || HAS_ERROR(error, ERROR_BIT_FLAGS::STRUCT_HASH_FAIL) || HAS_ERROR(error, ERROR_BIT_FLAGS::STRUCT_CANARY)) {
        fputc('\n', stream);
        return;
    }
    
    fprintf(stream, ":\n");

    char *true_pointer = ((char *)(stack -> data)) - sizeof(CanaryType);
    fprintf(stream, "\t\tCanary: %0llx\n", *(CanaryType *) true_pointer);

    for(StackSize i = 0; i < stack -> capacity; i++) {
        fprintf(stream, "\t\t[%03lld] ", i); // object index

        fprintf(stream, OBJECT_TO_STR, (stack -> data)[i]); // print value function (possible macros)

        if ((stack -> data)[i] == POISON_VALUE) fprintf(stream, " (POISON VALUE)"); // poison value warning
            
        fputc('\n', stream); // new line
    }

    fprintf(stream, "\t\tCanary: %0llx\n", *(CanaryType *) (true_pointer + sizeof(CanaryType) + stack -> capacity * sizeof(Object)));

    fputc('\n', stream);
}


void print_errors(ErrorBits error, FILE *stream) {
    if (error == ERROR_BIT_FLAGS::STACK_OK) {
        fprintf(stream, "\tOk\n");
        return;
    }
    else {
        fputc('\t', stream);
        print_binary(error, stream);
        fputc('\n', stream);
    }

    for(ErrorBits i = 0; i < 16; i++) {
        if (error & 1)
            fprintf(stream, "\t[ERROR] %s\n", ERROR_DESCRIPTION[i]);
        error >>= 1;
    }
}


static void print_binary(ErrorBits n, FILE *stream) {
    ErrorBits k = 1ull << 15;
    while(k > 0) {
        fputc(((n & k) > 0) + '0', stream);
        k = k >> 1;
    }
}


#if (PROTECT_LEVEL & HASH_PROTECT)

static ErrorBits check_struct_hash(Stack *stack) {
    CHECK(right_pointer(stack, sizeof(Stack)), return ERROR_BIT_FLAGS::INVALID_POINTER);

    ErrorBits error = ERROR_BIT_FLAGS::STACK_OK;

    HashType h1 = stack -> struct_hash, h2 = stack -> buffer_hash;

    stack -> struct_hash = 0;
    stack -> buffer_hash = 0;

    CHECK(gnu_hash(stack, sizeof(Stack)) == h1, error += ERROR_BIT_FLAGS::STRUCT_HASH_FAIL);

    stack -> struct_hash = h1;
    stack -> buffer_hash = h2;

    return error;
}


static void set_hash(Stack *stack) {
    CHECK(stack, return);

    stack -> struct_hash = 0;
    stack -> buffer_hash = 0;

    stack -> struct_hash = gnu_hash(stack, sizeof(Stack));
    stack -> buffer_hash = gnu_hash(stack -> data, stack -> size * sizeof(Object));
}


static HashType gnu_hash(void *ptr, size_t size) {
    HashType hash = 5381;

    for(size_t i = 0; i < size; i++)
        hash = hash * 33 + ((char *)(ptr))[i];

    return hash;
}

#endif

static int right_pointer(void *ptr, size_t size) {
    if (!ptr) return 0;

    MEMORY_BASIC_INFORMATION info = {};

    if (VirtualQuery(ptr, &info, sizeof(info))) {
        if (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return 0;

        DWORD mask = PAGE_READWRITE;

        return info.Protect & mask;
    }

    return 0;
}

