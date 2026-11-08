/*
 * test_student.c -- CS 313 P3: your three hand-traced tests.
 *
 * Name: Hadi Almarzooq
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
    
    /* Trace: Insert keys to check preorder.
             5
           /   \
          3     7
         / \
        2   4
        Preorder (Root, Left, Right): 5, 3, 2, 4, 7
    */
    bst_insert(t, 5);
    bst_insert(t, 3);
    bst_insert(t, 7);
    bst_insert(t, 2);
    bst_insert(t, 4);

    CHECK_PREORDER(t, 5, 3, 2, 4, 7);
    
    bst_destroy(t);
}

void student_test_2(void) {
    bst *t = bst_create();
    
    /* Trace: Insert keys, delete a node (30) with two children 
       whose successor (35) is NOT its direct right child (40).
       Initial:
             10
            /  \
           5    30
               /  \
             20    40
                  /  \
                 35   45
       After delete(30), 35 replaces it:
             10
            /  \
           5    35
               /  \
             20    40
                     \
                      45
        Preorder (Root, Left, Right): 10, 5, 35, 20, 40, 45
    */
    bst_insert(t, 10);
    bst_insert(t, 5);
    bst_insert(t, 30);
    bst_insert(t, 20);
    bst_insert(t, 40);
    bst_insert(t, 35);
    bst_insert(t, 45);
    
    bst_delete(t, 30);

    CHECK_PREORDER(t, 10, 5, 35, 20, 40, 45);
    
    bst_destroy(t);
}

void student_test_3(void) {
    bst *t = bst_create();
    
    /* Trace: Insert keys, delete the root (20), then check 
       postorder and height.
       Initial:
             20
            /  \
          10    30
         /  \     \
        5   15     35
       After delete(20), successor 30 replaces it:
             30
            /  \
          10    35
         /  \
        5   15
       Postorder (Left, Right, Root): 5, 15, 10, 35, 30
       Height: 2
    */
    bst_insert(t, 20);
    bst_insert(t, 10);
    bst_insert(t, 30);
    bst_insert(t, 5);
    bst_insert(t, 15);
    bst_insert(t, 35);
    
    bst_delete(t, 20);

    CHECK_POSTORDER(t, 5, 15, 10, 35, 30);
    CHECK(bst_height(t) == 2);
    
    bst_destroy(t);
}