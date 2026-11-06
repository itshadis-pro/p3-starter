/*
 * check.h -- tiny test framework for test_student.c. Do not edit.
 * The autograder uses its own copy.
 *
 *   CHECK(condition)            fails the test if condition is false.
 *   CHECK_PREORDER(t, k1, k2, ...)
 *   CHECK_INORDER(t, k1, k2, ...)
 *   CHECK_POSTORDER(t, k1, k2, ...)
 *                               fails the test unless that traversal of t
 *                               is exactly k1, k2, ... The keys must be
 *                               integer literals you worked out by hand,
 *                               such as 8, 3, 1 (not variables or
 *                               expressions).
 *
 * A test passes if every check passes AND it contains at least one
 * CHECK_PREORDER or CHECK_POSTORDER with 5 or more keys. (Inorder alone
 * does not count: it is always sorted, so it does not show the shape.)
 *
 * Run all three tests:  ./test_student
 * Run one test:         ./test_student 2
 */
#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "bst.h"

static int check_count = 0;
static int check_failures = 0;
static int check_shape_ok = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        check_count++;                                                  \
        if (!(cond)) {                                                  \
            printf("    FAIL line %d: CHECK(%s)\n", __LINE__, #cond);   \
            check_failures++;                                           \
        }                                                               \
    } while (0)

#define CHECK_KEYS_(t, fn, label, shape, ...)                           \
    check_keys_impl((t), (fn), (label), (shape), #__VA_ARGS__,          \
                    (const int[]){__VA_ARGS__},                         \
                    (int)(sizeof((int[]){__VA_ARGS__}) / sizeof(int)),  \
                    __LINE__)

#define CHECK_PREORDER(t, ...)  CHECK_KEYS_(t, bst_preorder, "preorder", 1, __VA_ARGS__)
#define CHECK_INORDER(t, ...)   CHECK_KEYS_(t, bst_inorder, "inorder", 0, __VA_ARGS__)
#define CHECK_POSTORDER(t, ...) CHECK_KEYS_(t, bst_postorder, "postorder", 1, __VA_ARGS__)

static inline void check_print_keys_(const int a[], int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf(i ? ", %d" : "%d", a[i]);
    }
    printf("]");
}

static inline void check_keys_impl(const bst *t,
                                   int (*fn)(const bst *, int[]),
                                   const char *label, int shape,
                                   const char *text, const int expected[],
                                   int n, int line) {
    check_count++;
    for (const char *p = text; *p != '\0'; p++) {
        if (!isdigit((unsigned char)*p) && *p != '-' && *p != ',' &&
            !isspace((unsigned char)*p)) {
            printf("    FAIL line %d: CHECK_", line);
            for (const char *q = label; *q != '\0'; q++) {
                putchar(toupper((unsigned char)*q));
            }
            printf(" needs integer literals you worked out by hand,"
                   " not \"%s\"\n", text);
            check_failures++;
            return;
        }
    }
    if (t == NULL) {
        printf("    FAIL line %d: tree is NULL\n", line);
        check_failures++;
        return;
    }
    int size = bst_size(t);
    int *got = calloc((size_t)(size > 0 ? size : 1) + 1000, sizeof *got);
    int m = fn(t, got);
    int ok = (m == n);
    for (int i = 0; ok && i < n; i++) {
        if (got[i] != expected[i]) {
            ok = 0;
        }
    }
    if (!ok) {
        printf("    FAIL line %d: %s\n      you expected ", line, label);
        check_print_keys_(expected, n);
        printf("\n      the tree gives ");
        if (m >= 0 && m <= size + 1000) {
            check_print_keys_(got, m);
        } else {
            printf("(count %d)", m);
        }
        printf("\n");
        check_failures++;
    } else if (shape && n >= 5) {
        check_shape_ok++;
    }
    free(got);
}

void student_test_1(void);
void student_test_2(void);
void student_test_3(void);

int main(int argc, char **argv) {
    void (*tests[3])(void) = {student_test_1, student_test_2, student_test_3};
    int first = 1, last = 3, all_pass = 1;

    if (argc > 1) {
        first = last = atoi(argv[1]);
        if (first < 1 || first > 3) {
            printf("usage: %s [1|2|3]\n", argv[0]);
            return 2;
        }
    }
    for (int t = first; t <= last; t++) {
        check_count = check_failures = check_shape_ok = 0;
        printf("student_test_%d:\n", t);
        tests[t - 1]();
        int pass = check_failures == 0 && check_shape_ok >= 1;
        if (check_failures == 0 && check_shape_ok == 0) {
            printf("    FAIL: no CHECK_PREORDER or CHECK_POSTORDER with 5 or"
                   " more keys passed\n");
        }
        printf("    %s (%d checks)\n", pass ? "PASS" : "FAIL", check_count);
        fflush(stdout);
        all_pass = all_pass && pass;
    }
    return all_pass ? 0 : 1;
}

#endif
