/**
 * \file
 * \brief Log module header
 * 
 * Contains log_file pointer and its setter functions
*/

#include <stdio.h>


/**
 * \brief Opens log file by its path
 * \param filename Path to the file
 * \return 0 - OK, 1 - FAIL
*/
int open_log(const char filename[]);


/**
 * \brief Returns log file
 * \return Log file or null
 * \note Will warn you if log file is null
*/
FILE *get_log_file(void);


/**
 * \brief Closes log file
 * \return 0 - OK, 1 - FAIL
 * \note In any case log file will be set to null
*/
int close_log(void);
