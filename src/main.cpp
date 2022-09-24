#include <stdio.h>
#include "stack.hpp"


int main() {
    Stack stack = {};

    stack_constructor(&stack, 10);

    for(int i = 1; i <= 11; i++)
        stack_push(&stack, i);
    
    for(int i = 0; i < 20; i++) {
        Object value = POISON_VALUE;
        stack_pop(&stack, &value);
        printf("%i ", value);
    }

    stack_destructor(&stack);

    return 0;
}
