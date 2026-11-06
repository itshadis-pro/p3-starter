/*
 * test_student.c -- CS 313 P3: your three hand-traced tests.
 *
 * Name:
 *
 * Write three tests. Each one inserts (and maybe deletes) some keys, then
 * checks a traversal with CHECK_PREORDER(t, ...) or CHECK_POSTORDER(t, ...),
 * listing keys YOU worked out by drawing the tree by hand before running
 * anything. Draw the tree in a comment above the check. Use a block
 * comment for drawings: a // line that ends in a backslash also comments
 * out the next line. You may add CHECK(...) lines for anything else
 * (bst_height, bst_successor, ...).
 *
 * The autograder runs your tests against the instructor's bst.c. A test
 * passes if all its checks pass there and it has at least one
 * CHECK_PREORDER or CHECK_POSTORDER with 5 or more keys. So a wrong hand
 * trace fails the test.
 *
 * Suggested tests (use your own keys, at least 7 of them):
 *   1. Insert keys, then check the preorder.
 *   2. Insert keys, delete a node with two children whose successor is
 *      NOT its right child, then check the preorder.
 *   3. Insert keys, delete the root, then check the postorder and the
 *      height.
 *
 * Worked example (not one of your three):
 *
 *   bst *t = bst_create();
 *   bst_insert(t, 5); bst_insert(t, 2); bst_insert(t, 8);
 *   bst_insert(t, 1); bst_insert(t, 9);
 *   (drawing, in a block comment:)
 *              5
 *             / \
 *            2   8
 *           /     \
 *          1       9
 *   CHECK_PREORDER(t, 5, 2, 1, 8, 9);
 *   CHECK(bst_height(t) == 2);
 *   bst_destroy(t);
 *
 * Build and run against your own bst.c:  make test
 */
#include "check.h"

void student_test_1(void) {
    bst *t = bst_create();
    /* TODO */
    bst_destroy(t);
}

void student_test_2(void) {
    bst *t = bst_create();
    /* TODO */
    bst_destroy(t);
}

void student_test_3(void) {
    bst *t = bst_create();
    /* TODO */
    bst_destroy(t);
}
