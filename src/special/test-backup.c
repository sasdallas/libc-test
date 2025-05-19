/**
 * @file src/common/test-backup.c
 * @brief Backup test
 * 
 * This test is run whenever compilation fails for another. When called, it simply errors out.
 * 
 * @copyright
 * No copyright has been placed on this file
 */

#include "../common/test.h"


#ifndef TEST_FILE_NAME
#error "Define TEST_FILE_NAME"
#endif

#define STR(x) #x
#define STRSTR(x) STR(x)

int main(int argc, char *argv[]) {
    t_error("Error running test \"%s\" (compilation failed during test suite build time)\n", STR(TEST_FILE_NAME));
    return 1;
}