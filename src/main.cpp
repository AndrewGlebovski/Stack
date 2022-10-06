#include "stack.hpp"
#include "logs.hpp"
#include "test.hpp"


ReturnCode test_normal(void *data); ///< Standart stack check. What could go wrong?
ReturnCode test_1001_push_and_pop(void *data); ///< Pushes 1001 element in stack, then pops it out
ReturnCode test_struct_hash(void *data); ///< Changes structure size to see how verificator would work
ReturnCode test_canary(void *data); ///< Changes structure canary value to see how verificator would work 


Test tests[] = {
    {
        &test_normal,
        ERROR_BIT_FLAGS::STACK_OK,
        nullptr
    },
    {
        &test_1001_push_and_pop,
        ERROR_BIT_FLAGS::STACK_OK,
        nullptr
    },
    {
        &test_struct_hash,
        ERROR_BIT_FLAGS::STRUCT_HASH_FAIL,
        nullptr
    },
    {
        &test_canary,
        ERROR_BIT_FLAGS::STRUCT_CANARY,
        nullptr
    }
};


int main() {
    open_log("log.txt");

    run_tests(tests, sizeof(tests) / sizeof(Test), stdin);

    close_log();

    return 0;
}




ReturnCode test_normal(void *data) {
    fprintf(get_log_file(), "\n~~~~~~~~~~~test_normal~~~~~~~~~~~~\n");

    Stack stack = {};

    stack_constructor(&stack, 10);

    for(int i = 1; i <= 10; i++)
        stack_push(&stack, i);

    for(int i = 1; i <= 5; i++) {
        Object value = 0;
        stack_pop(&stack, &value);
    }

    return stack_destructor(&stack);
}


ReturnCode test_1001_push_and_pop(void *data) {
    fprintf(get_log_file(), "\n~~~~~~test_1001_push_and_pop~~~~~~\n");

    Stack stack = {};

    stack_constructor(&stack, 10);

    for(int i = 1; i <= 1001; i++)
        stack_push(&stack, i);

    for(int i = 1; i <= 1001; i++) {
        Object value = 0;
        stack_pop(&stack, &value);
    }

    return stack_destructor(&stack);
}


ReturnCode test_struct_hash(void *data) {
    fprintf(get_log_file(), "\n~~~~~~~~~test_struct_hash~~~~~~~~~\n");

    Stack stack = {};

    stack_constructor(&stack, 10);

    for(int i = 1; i <= 1001; i++)
        stack_push(&stack, i);

    stack.size = (StackSize) 100000;

    return stack_destructor(&stack);
}


ReturnCode test_canary(void *data) {
    fprintf(get_log_file(), "\n~~~~~~~~~~~~test_canary~~~~~~~~~~~\n");

    Stack stack = {};

    stack_constructor(&stack, 10);

    for(int i = 1; i <= 1001; i++)
        stack_push(&stack, i);

    stack.canary_begin = (CanaryType) 100000;

    return stack_destructor(&stack);
}
