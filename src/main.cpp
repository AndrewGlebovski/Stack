#include <stdio.h>
#include "stack.hpp"


int main() {
    Stack stack = {};

    stack_constructor(&stack, 10);

    for(int i = 1; i <= 12; i++)
        stack_push(&stack, i);

    for(int i = 1; i <= 8; i++) {
        Object value = 0;
        stack_pop(&stack, &value);
    }

    stack_dump(&stack, stack_check(&stack));

    stack.capacity = 15;

    stack_dump(&stack, stack_check(&stack));

    stack_destructor(&stack);

    return 0;
}
