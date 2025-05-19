/**
 * @file src/special/test-runner.c
 * @brief Main test runner program
 * 
 * 
 * @copyright
 * This file is part of the Ethereal Operating System.
 * It is released under the terms of the BSD 3-clause license.
 * Please see the LICENSE file in the main repository for more details.
 * 
 * Copyright (C) 2025 Samuel Stuart
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include "test.h"
#include <fcntl.h>
 
static char *dir = NULL;

static int total_tests = 0;
static int failed_tests = 0;

/* ugly */
#define EXECUTE_TEST(testname) tmp = strcat(dir, "/" testname); \
                                total_tests++; \
                                if (run_test(tmp, testname)) failed_tests++;

void print_version() {
    printf("libc-test v1.0.0\n");
    printf("originally developed for the musl libc project\n");
}

int run_test(char *test_path, char *test_name) {
    t_printf("[START:%s] Starting test\n", test_name);

    pid_t p = fork();
    if (p < 0) {
        fprintf(stderr, "test: fork: %s\n", strerror(errno));
        exit(1);
    }
    
    if (!p) {
        const char *argv[] = { test_path, NULL };
        execvp(test_path, argv);
        t_error("execvp failed for test\n");
        exit(EXIT_FAILURE);
    } else {
        int status;
        if (waitpid(p, &status, 0) != p) {
            t_error("CRITICAL: waitpid failed: %s\n", strerror(errno));
            t_error("CRITICAL: This is not a bug in a test - this is a bug in your operating system\n");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == EXIT_SUCCESS) {
                t_printf("[SUCCESS:%s] Process exited with status 0\n", test_name);
                return t_status;
            }
            t_printf("[FAIL:%s] Process exited with status %d\n", test_name, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            t_printf("[FAIL:%s] Process was signalled\n", test_name);
        } else {
            t_printf("[FAIL:%s] Unknown error, process did not exit and was not signalled\n", test_name);
        }

        return 1;
    }

    __builtin_unreachable();
}


void run_functional_tests() {
    char *tmp;
    EXECUTE_TEST("argv");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: test <test directory> <optional:device to reroute stdout to>\n");
        return 1;
    }

    print_version();

    // Set directory
    dir = argv[1];

    // Redirect if needed
    if (argc > 2) {
        printf("redirecting stdout to %s\n", argv[2]);
        int f = open(argv[2], O_RDWR);
        if (f < 0) {
            fprintf(stderr, "test: %s: %s\n", argv[2], strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (dup2(f, STDOUT_FILENO) < 0) {
            fprintf(stderr, "test: dup2: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    run_functional_tests();
    return t_status;
}