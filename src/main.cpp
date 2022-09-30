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

    stack.capacity = 50000000000;
    stack_push(&stack, 5);

    stack_destructor(&stack);

    return 0;
}
