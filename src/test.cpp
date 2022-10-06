/**
 * \file
 * \brief Test module source
 * 
 * Contains realisation of run_tests() function
*/

#include <stdio.h>
#include "test.hpp"


int run_tests(Test tests[], unsigned long long size, void *data) {
    int count = 0;

    for(size_t t = 0; t < size; t++) {
        ReturnCode return_code = (*tests[t].test_func)(data);

        if (return_code == tests[t].return_code) {
            printf("Test %llu: Ok\n", t + 1);
            count++;
        }
        else {
            printf("Test %llu: Fail, Expected: %llu, Got: %llu\n", t + 1, tests[t].return_code, return_code);
        }
    }

    return count;
}
