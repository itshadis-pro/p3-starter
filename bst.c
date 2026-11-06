/*
 * bst.c -- CS 313 P3: binary search tree with parent pointers.
 *
 * Name:
 *
 * Fill in every TODO. The rules are in bst.h and README.md.
 * This skeleton compiles, but it fails the tests until you finish it.
 * Follow CLRS 4e Chapter 12; its pseudocode maps almost line for line to C
 * (NIL becomes NULL, T.root becomes t->root, x.left becomes x->left).
 * You may add your own static helper functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include "bst.h"

/* ---------- provided ---------- */

bst *bst_create(void) {
    bst *t = malloc(sizeof *t);
    if (t != NULL) {
        t->root = NULL;
    }
    return t;
}

static void print_rec(const bst_node *x, int depth) {
    if (x == NULL) {
        return;
    }
    print_rec(x->right, depth + 1);
    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("%d\n", x->key);
    print_rec(x->left, depth + 1);
}

/* Prints the tree turned 90 degrees counterclockwise: the root is at the
 * left edge, right subtrees are above their parent, left subtrees below. */
void bst_print(const bst *t) {
    if (t == NULL || t->root == NULL) {
        printf("(empty tree)\n");
        return;
    }
    print_rec(t->root, 0);
}

/* ---------- your code ---------- */

void bst_destroy(bst *t) {
    /* TODO: free every node, then free t. Do nothing if t is NULL.
     * Hint: a recursive helper that frees a node's two subtrees and then
     * the node itself (a postorder walk). */
    (void)t;
}

void bst_insert(bst *t, int key) {
    /* TODO: CLRS TREE-INSERT. malloc a node, set all four fields, walk
     * down from the root to find its parent, and link it in.
     * Duplicates go right: go left only if key < x->key. */
    (void)t;
    (void)key;
}

bst_node *bst_find(const bst *t, int key) {
    /* TODO: CLRS ITERATIVE-TREE-SEARCH starting at t->root. */
    (void)t;
    (void)key;
    return NULL;
}

bool bst_contains(const bst *t, int key) {
    /* TODO */
    (void)t;
    (void)key;
    return false;
}

bool bst_min(const bst *t, int *out) {
    /* TODO: CLRS TREE-MINIMUM. Return false if the tree is empty. */
    (void)t;
    (void)out;
    return false;
}

bool bst_max(const bst *t, int *out) {
    /* TODO: CLRS TREE-MAXIMUM. Return false if the tree is empty. */
    (void)t;
    (void)out;
    return false;
}

bool bst_successor(const bst *t, int key, int *out) {
    /* TODO: find the node, then CLRS TREE-SUCCESSOR. Two cases:
     * it has a right subtree, or you walk up through parent pointers.
     * Return false if key is missing or if its node has no successor
     * (it is the last node in an inorder walk). */
    (void)t;
    (void)key;
    (void)out;
    return false;
}

void bst_transplant(bst *t, bst_node *u, bst_node *v) {
    /* TODO: CLRS TRANSPLANT. Remember v may be NULL. */
    (void)t;
    (void)u;
    (void)v;
}

bool bst_delete(bst *t, int key) {
    /* TODO: CLRS TREE-DELETE using bst_transplant, then free the node.
     * Return false (tree unchanged) if key is missing. */
    (void)t;
    (void)key;
    return false;
}

int bst_preorder(const bst *t, int out[]) {
    /* TODO: recursive or iterative; see README.md. Return the count. */
    (void)t;
    (void)out;
    return -1;
}

int bst_inorder(const bst *t, int out[]) {
    /* TODO */
    (void)t;
    (void)out;
    return -1;
}

int bst_postorder(const bst *t, int out[]) {
    /* TODO */
    (void)t;
    (void)out;
    return -1;
}

int bst_size(const bst *t) {
    /* TODO: see the worked example in README.md. */
    (void)t;
    return -1;
}

int bst_height(const bst *t) {
    /* TODO: empty tree -1, one node 0. */
    (void)t;
    return -2;
}
