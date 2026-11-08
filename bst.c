/*
 * bst.c -- CS 313 P3: binary search tree with parent pointers.
 *
 * Name: Hadi Almarzooq
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

static void destroy_rec(bst_node *x) {
    if (x == NULL){
        return;
    }

    // 1. recursively free the left and right subtrees
    destroy_rec(x->left);
    destroy_rec(x->right);

    // 2. free the current node
    free(x);
}

void bst_destroy(bst *t) {
    /* TODO: free every node, then free t. Do nothing if t is NULL.
     * Hint: a recursive helper that frees a node's two subtrees and then
     * the node itself (a postorder walk). */
    if (t == NULL){
        return;
    }

    // 3. call the helper starting at the root
    destroy_rec(t->root);

    // 4. Free the main tree struct
    free(t);
 }

void bst_insert(bst *t, int key) {
    // 1. Allocate and initialize the new leaf node
    bst_node *z = malloc(sizeof(bst_node));
    if (z == NULL) {
        return;
    }
    z->key = key;
    z->left = NULL;
    z->right = NULL;  // Fixed: lowercase 'z', and use '->'
    
    bst_node *y = NULL;    // Fixed: added the missing '=' sign
    bst_node *x = t->root;

    // 2. Walk down the tree to find the empty spot
    while (x != NULL) {
        y = x; // Fixed: lowercase 'y' and 'x'. Save current node as parent before moving down.
        if (key < x->key) {
            x = x->left;   // Strictly smaller keys go left
        } else {
            x = x->right;  // Duplicates and larger keys go right
        }
    }

    // 3. Link the new node 'z' UP to its parent 'y'
    z->parent = y; // Link up to 'y' (the parent). 'x' is now NULL because the while loop finished.

    // 4. Link the new parent 'y' DOWN to 'z' (or set as root if tree was empty)
    if (y == NULL) {
        t->root = z; // Tree was empty, so 'z' is the new root
    } else if (key < y->key) {
        y->left = z; // 'z' is smaller than its parent, so it becomes the left child
    } else {
        y->right = z; // 'z' is greater than or equal to its parent, so it becomes the right child
    }
}

bst_node *bst_find(const bst *t, int key) {
    // Start at the root
    bst_node *x = t->root;
    
    // Walk down until we hit NULL or find the exact key
    while (x != NULL && key != x->key) {
        if (key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    
    // Returns the node if found, or NULL if the while loop ran off the bottom
    return x;
}

bool bst_contains(const bst *t, int key) {
    // If find returns a valid node, the tree contains the key. Otherwise, it doesn't.
    return bst_find(t, key) != NULL;
}

bool bst_min(const bst *t, int *out) {
    // 1. Rule: return false if the tree is empty
    if (t == NULL || t->root == NULL) {
        return false;
    }
    
    bst_node *x = t->root;
    
    // 2. Go as far left as possible
    while (x->left != NULL) {
        x = x->left;
    }
    
    // 3. Save the minimum key and return true
    *out = x->key;
    return true;
}

bool bst_max(const bst *t, int *out) {
    if (t == NULL || t->root == NULL) {
        return false;
    }
    
    bst_node *x = t->root;
    
    // 4. Go as far right as possible
    while (x->right != NULL) {
        x = x->right;
    }
    
    *out = x->key;
    return true;
}

bool bst_successor(const bst *t, int key, int *out) {
    bst_node *x = bst_find(t, key);
    if (x == NULL) return false;
    
    // Case 1: Node has a right subtree (find the minimum of that right subtree)
    if (x->right != NULL) {
        bst_node *y = x->right;
        while (y->left != NULL) {
            y = y->left;
        }
        *out = y->key;
        return true;
    }
    
    // Case 2: Walk up the parent pointers
    bst_node *y = x->parent;
    while (y != NULL && x == y->right) {
        x = y;
        y = y->parent;
    }
    
    // If y is NULL, x was the absolute maximum of the tree, so no successor exists
    if (y == NULL) return false;
    
    *out = y->key;
    return true;
}

void bst_transplant(bst *t, bst_node *u, bst_node *v) {
    if (u->parent == NULL) {
        t->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    
    if (v != NULL) {
        v->parent = u->parent;
    }
}

bool bst_delete(bst *t, int key) {
    bst_node *z = bst_find(t, key);
    if (z == NULL) return false;

    // Case 1 & 2: Node has at most one child
    if (z->left == NULL) {
        bst_transplant(t, z, z->right);
    } else if (z->right == NULL) {
        bst_transplant(t, z, z->left);
    } else {
        // Case 3: Node has two children. Find the successor 'y'
        bst_node *y = z->right;
        while (y->left != NULL) {
            y = y->left;
        }
        
        // If 'y' is not the direct right child of 'z', replace 'y' with its own right child
        if (y->parent != z) {
            bst_transplant(t, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        
        // Replace 'z' with 'y'
        bst_transplant(t, z, y);
        y->left = z->left;
        y->left->parent = y;
    }
    
    free(z);
    return true;
}

// 1. Preorder (Root, Left, Right)
static int pre_rec(const bst_node *x, int out[], int i) {
    if (x == NULL) return i;
    out[i] = x->key;
    i++;
    i = pre_rec(x->left, out, i);
    return pre_rec(x->right, out, i);
}

int bst_preorder(const bst *t, int out[]) {
    if (t == NULL) return 0;
    return pre_rec(t->root, out, 0);
}

// 2. Inorder (Left, Root, Right)
static int in_rec(const bst_node *x, int out[], int i) {
    if (x == NULL) return i;
    i = in_rec(x->left, out, i);
    out[i] = x->key;
    i++;
    return in_rec(x->right, out, i);
}

int bst_inorder(const bst *t, int out[]) {
    if (t == NULL) return 0;
    return in_rec(t->root, out, 0);
}

// 3. Postorder (Left, Right, Root)
static int post_rec(const bst_node *x, int out[], int i) {
    if (x == NULL) return i;
    i = post_rec(x->left, out, i);
    i = post_rec(x->right, out, i);
    out[i] = x->key;
    i++;
    return i;
}

int bst_postorder(const bst *t, int out[]) {
    if (t == NULL) return 0;
    return post_rec(t->root, out, 0);
}

static int size_rec(const bst_node *x) {
    if (x == NULL) {
        return 0;
    }
    return 1 + size_rec(x->left) + size_rec(x->right);
}

int bst_size(const bst *t) {
    if (t == NULL) return 0;
    return size_rec(t->root);
}

static int height_rec(const bst_node *x) {
    // Fixed: The rule for an empty tree is -1
    if (x == NULL) {
        return -1;
    }
    
    int left_h = height_rec(x->left);
    int right_h = height_rec(x->right);
    
    // Fixed: Use the integer variables left_h and right_h, not the pointers
    if (left_h > right_h) {
        return 1 + left_h;
    } else {
        return 1 + right_h;
    }
}


int bst_height(const bst *t) {
    if (t == NULL) {
        return -1;
    }
    
    // 4. Start the recursion at the top of the tree
    return height_rec(t->root);
}