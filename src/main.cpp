#include <stdio.h>
#include "stack.hpp"
#include "logs.hpp"


int main() {
    open_log("log.txt");

    Stack stack = {};

    stack_constructor(&stack, 10);

    for(int i = 1; i <= 10; i++)
        stack_push(&stack, i);

    for(int i = 1; i <= 8; i++) {
        Object value = 0;
        stack_pop(&stack, &value);
    }

    //stack.size = 100;
    //stack.capacity = 100;
    //stack.data = nullptr;

    stack.data[0] = 0;

    stack_destructor(&stack);

    close_log();

    return 0;
}
