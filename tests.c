#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "libdo.h"

#define MAX_LOC_DIGITS  "3"
#define TEST(MSG, COND) \
do { \
    printf("%s:%-"MAX_LOC_DIGITS"d : | \033[0;35m%-40s\033[0m | \033[0;33m%-40s\033[0m |", \
            __FILE__, __LINE__, MSG, "{ "#COND" }"); \
    if (COND) { puts("  \033[1;32mOK\033[0m  |"); tests_passed++; } \
    else { puts(" \033[1;31mFAIL\033[0m |"); tests_failed++; } \
} while (0)

#define LOG(MSG) \
do { \
    printf("%s:%-"MAX_LOC_DIGITS"d : \033[7;34m%s\033[0m\n", __FILE__, __LINE__, MSG); \
} while (0)

void test_bool_ptr_predicate(struct do_doer *doer);

void test_func_predicate(struct do_doer *doer);

void test_time_predicate(struct do_doer *doer);

void test_periodic_with_expiry(struct do_doer *doer);

void test_priorities(struct do_doer *doer);

static int tests_passed;
static int tests_failed;
static int runs;


int main() {
    struct do_doer *doer;
    LOG("--- Test begin ---");
    doer = do_init();
    TEST("Doer init", doer);
    if (!doer) {
        exit(EXIT_FAILURE);
    }
    test_bool_ptr_predicate(doer);
    test_func_predicate(doer);
    test_time_predicate(doer);
    test_periodic_with_expiry(doer);
    test_priorities(doer);
    do_destroy(doer);
    exit(EXIT_SUCCESS);
}

bool work1_func(void *data) {
    runs++;
    return *((bool *) data);
}

void test_bool_ptr_predicate(struct do_doer *doer) {
    bool run_work1 = false, remove_work1 = false;
    struct do_work *work1 = do_work_if(work1_func, NULL, &run_work1);
    LOG("--- Test bool ptr predicate ---");
    runs = 0;
    TEST("Work-1 init with bool ptr predicate", work1);
    do_work_set_data(work1, &remove_work1);
    TEST("Work-1 added to doer", do_so(doer, work1));
    run_work1 = true;
    remove_work1 = false;
    TEST("Work-1 runs", do_loop(doer));
    TEST("Work-1 runs again", do_loop(doer));
    remove_work1 = true;
    TEST("Work-1 runs and is removed", !do_loop(doer));
    TEST("Work-1 ran thrice in total", runs == 3);
}

bool work2_func(void *data) {
    (void) data;
    runs++;
    return false;
}

bool work2_predicate(void *data) {
    return *((bool *) data);
}

void test_func_predicate(struct do_doer *doer) {
    bool run_work2 = false;
    struct do_work *work2 = do_work_when(work2_func, NULL, work2_predicate);
    LOG("--- Test func predicate ---");
    runs = 0;
    TEST("Work-2 init with func predicate", work2);
    do_work_set_data(work2, &run_work2);
    TEST("Work-2 added to doer", do_so(doer, work2));
    TEST("Work-2 doesn't run", do_loop(doer));
    run_work2 = true;
    TEST("Work-2 runs", do_loop(doer));
    do_not_do(doer, work2);
    TEST("Work-2 runs and is removed", !do_loop(doer));
}

bool work3_func(void *data) {
    (void) data;
    return true;
}

bool work4_func(void *data) {
    runs++;
    do_work_set_predicate_time(data, time(NULL) + 1);
    return false;
}

void test_time_predicate(struct do_doer *doer) {
    int sec = 2;
    time_t add_tm = time(NULL);
    struct do_work *work3 = do_work_after(work3_func, NULL, add_tm + sec);
    LOG("--- Test time predicate ---");
    TEST("Work-3 init with time predicate", work3);
    TEST("Work-3 added to doer", do_so(doer, work3));
    while (do_loop(doer));
    TEST("Work-3 ran after specified time", time(NULL) == (add_tm + sec));
}

void test_periodic_with_expiry(struct do_doer *doer) {
    int until_sec = 3;
    time_t add_tm = time(NULL);
    time_t until_tm = add_tm + until_sec;
    struct do_work *work4 = do_work_after(work4_func, NULL, add_tm + 1);
    LOG("--- Test periodic work with expiry ---");
    TEST("Work-4 init with time predicate", work4);
    do_work_set_data(work4, work4);
    TEST("Work-4 added to doer", do_so_until(doer, work4, until_tm));
    while (do_loop(doer));
    TEST("Work-4 ran until specified seconds", time(NULL) == until_tm);
    TEST("Work-4 ran expected number of times", runs == until_sec);
}

void test_priorities(struct do_doer *doer) {
    (void) doer;
}
