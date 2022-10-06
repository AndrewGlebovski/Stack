/**
 * \file
 * \brief Test module header
 * 
 * Include it to run indepent test function
*/


/// Return code type (good for bit flags or integer)
typedef unsigned long long ReturnCode;


/// Test function template
typedef ReturnCode (*TestFunc)(void *data);


/// Contains test function, argument for it and its expected return code
typedef struct {
    TestFunc test_func; ///< This function will be run for testing
    ReturnCode return_code = 0; ///< Expected exit code for test function
    void *data = nullptr; ///< Additional data for testing
} Test;


/**
 * \brief Runs tests from array
 * \param tests Array of tests to run
 * \param size Size of array
 * \param data Some additional data for test
 * \return Number of successfull tests
*/
int run_tests(Test tests[], unsigned long long size, void *data);
