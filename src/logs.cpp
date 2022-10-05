#include "logs.hpp"


FILE *log_file = nullptr;


int open_log(const char filename[]) {
    log_file = fopen(filename, "w");

    if (!log_file) {
        printf("Couldn't open log file %s!\n", filename);
        return 1;
    }

    setvbuf(log_file, NULL, _IOFBF, 512);

    return 0;
}


FILE *get_log_file(void) {
    if (!log_file) printf("No log file!\n");
    return log_file;
}


int close_log(void) {
    if (log_file) {
        fclose(log_file);
        return 0;
    }
    else {
        printf("No log file to close!\n");
        return 1;
    }
}
