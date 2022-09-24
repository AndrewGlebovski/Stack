#include <stdlib.h>

#define POISON_VALUE 0xC0FFEE


const float STACK_FACTOR = 2.0;


typedef int Object;
typedef size_t StackSize;


typedef struct Stack
{
    Object *data = NULL;
    StackSize size = 0;
    StackSize capacity = 0;
} Stack;


enum EXIT_CODE {
    OK = 0,
    ERROR = 1
};


EXIT_CODE stack_constructor(Stack *stack, StackSize capacity);


EXIT_CODE stack_resize(Stack *stack, StackSize capacity);


EXIT_CODE stack_push(Stack *stack, Object object);


EXIT_CODE stack_pop(Stack *stack, Object *object);


EXIT_CODE stack_destructor(Stack *stack);




EXIT_CODE stack_constructor(Stack *stack, StackSize capacity) {
    stack -> data = (Object *) calloc(capacity, sizeof(Object));

    for(StackSize i = 0; i < capacity ; i++)
        (stack -> data)[i] = POISON_VALUE;
    
    stack -> capacity = capacity;
    stack -> size = 0;

    return OK;
}


EXIT_CODE stack_resize(Stack *stack, StackSize capacity) {
    stack -> data = (Object *) realloc(stack -> data, capacity * sizeof(Object));

    for(StackSize i = stack -> capacity; i < capacity ; i++)
        (stack -> data)[i] = POISON_VALUE;
    
    stack -> capacity = capacity;

    return OK;
}


EXIT_CODE stack_push(Stack *stack, Object object) {
    if ((stack -> size) + 1 > stack -> capacity)
        stack_resize(stack, (StackSize) ((float)(stack -> capacity) * STACK_FACTOR));

    (stack -> data)[(stack -> size)++] = object;

    return OK;
}


EXIT_CODE stack_pop(Stack *stack, Object *object) {
    if (stack -> size == 0) return ERROR;

    *object = (stack -> data)[--(stack -> size)];
    (stack -> data)[(stack -> size)] = POISON_VALUE;

    if (((stack -> size) < (StackSize) ((float)(stack -> capacity) / (2 * STACK_FACTOR))) && stack -> size > 10)
        stack_resize(stack, (StackSize) ((float)(stack -> capacity) / STACK_FACTOR));

    return OK;
}


EXIT_CODE stack_destructor(Stack *stack) {
    free(stack -> data);
    stack -> data = NULL;
    
    stack -> capacity = 0;
    stack -> size = 0;

    return OK;
}
