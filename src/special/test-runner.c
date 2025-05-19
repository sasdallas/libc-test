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
#define EXECUTE_TEST(testname) snprintf(tmp, 512, "%s/%s", dir, testname); \
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
                return 0;
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
    char tmp[512];
    EXECUTE_TEST("argv");
    EXECUTE_TEST("basename");
    EXECUTE_TEST("clocale_mbfuncs");
    EXECUTE_TEST("clock_gettime");
    EXECUTE_TEST("crypt");
    EXECUTE_TEST("dirname");
    EXECUTE_TEST("dlopen_dso");
    EXECUTE_TEST("dlopen");
    EXECUTE_TEST("env");
    EXECUTE_TEST("fcntl");
    EXECUTE_TEST("fdopen");
    EXECUTE_TEST("fnmatch");
    EXECUTE_TEST("fscanf");
    EXECUTE_TEST("fwscanf");
    EXECUTE_TEST("iconv_open");
    EXECUTE_TEST("inet_pton");
    EXECUTE_TEST("ipc_msg");
    EXECUTE_TEST("ipc_sem");
    EXECUTE_TEST("ipc_shm");
    EXECUTE_TEST("mbc");
    EXECUTE_TEST("memstream");
    EXECUTE_TEST("mntent");
    EXECUTE_TEST("popen");
    EXECUTE_TEST("pthread_cancel-points");
    EXECUTE_TEST("pthread_cancel");
    EXECUTE_TEST("pthread_cond");
    EXECUTE_TEST("pthread_mutex_pi");
    EXECUTE_TEST("pthread_mutex");
    EXECUTE_TEST("pthread_robust");
    EXECUTE_TEST("pthread_tsd");
    EXECUTE_TEST("qsort");
    EXECUTE_TEST("random");
    EXECUTE_TEST("search_hsearch");
    EXECUTE_TEST("search_insque");
    EXECUTE_TEST("search_lsearch");
    EXECUTE_TEST("search_tsearch");
    EXECUTE_TEST("sem_init");
    EXECUTE_TEST("sem_open");
    EXECUTE_TEST("setjmp");
    EXECUTE_TEST("snprintf");
    EXECUTE_TEST("socket");
    EXECUTE_TEST("spawn");
    EXECUTE_TEST("sscanf_long");
    EXECUTE_TEST("sscanf");
    EXECUTE_TEST("stat");
    EXECUTE_TEST("strftime");
    EXECUTE_TEST("string_memcpy");
    EXECUTE_TEST("string_memmem");
    EXECUTE_TEST("string_memset");
    EXECUTE_TEST("string_strchr");
    EXECUTE_TEST("string_strcspn");
    EXECUTE_TEST("string_strstr");
    EXECUTE_TEST("string");
    EXECUTE_TEST("strptime");
    EXECUTE_TEST("strtod_long");
    EXECUTE_TEST("strtod_simple");
    EXECUTE_TEST("strtod");
    EXECUTE_TEST("strtof");
    EXECUTE_TEST("strtol");
    EXECUTE_TEST("strtold");
    EXECUTE_TEST("swprintf");
    EXECUTE_TEST("tgmath");
    EXECUTE_TEST("time");
    EXECUTE_TEST("udiv");
    EXECUTE_TEST("ungetc");
    EXECUTE_TEST("utime");
    EXECUTE_TEST("vfork");
    EXECUTE_TEST("wcsstr");
    EXECUTE_TEST("wcstol");
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

    t_printf("[STATISTICS] Finished running all tests\n");
    t_printf("[STATISTICS] Total %d tests - %d failed, %d succeeded\n", total_tests, failed_tests, (total_tests - failed_tests));

    return t_status;
}