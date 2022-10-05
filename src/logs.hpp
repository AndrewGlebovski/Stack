/**
 * \file
 * \brief Contains log and assert macros
*/

#include <stdio.h>


/**
 * \brief This assert throw errors to log file
 * \param [in] condition Condition to check
 * \param [in] error This error code will be printed if condition fails
 * \param [in] msg This message will be printed if condition fails
 * \param [in] action This code will be executed if condition fails
*/
#define LOG_ON_ERROR(condition, error, msg, action) \
do {\
    if (!LOG_FILE) \
        fprintf(stderr, "No log file"); \
    \
    else if (!(condition)) { \
        fprintf(LOG_FILE, "%s in function %s in line %d\n[%.3d] %s\n\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, error, msg); \
        fflush(LOG_FILE); \
        action; \
    } \
} while(0)


int open_log(const char filename[]);


int close_log(void);
