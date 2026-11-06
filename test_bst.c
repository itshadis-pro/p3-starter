/*
 * test_bst.c -- CS 313 P3 autograder tests. Gradescope runs this same
 * file, so a clean local run means a clean Gradescope run (apart from
 * valgrind, your three student tests, and the REPORT.md check).
 *
 *   ./test_bst             run every test, each in its own process
 *   ./test_bst NAME        run one test here (exit 0 and "passed" = pass);
 *                          use this with a debugger
 *   ./test_bst --list      print the test names
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bst.h"

/* ------------------------------------------------------------------ */
/* Checking helpers. Each check prints what went wrong, returns false.  */
/* ------------------------------------------------------------------ */

#define LEN(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define REQUIRE(check) do { if (!(check)) return false; } while (0)

static int tu_checks = 0;
static int tu_failures = 0;
static bool record(bool ok)
{
    tu_checks++;
    if (!ok)
        tu_failures++;
    return ok;
}

static bool check_int(const char *what, int got, int expected)
{
    if (got != expected)
        printf("  FAIL %s: got %d, expected %d\n", what, got, expected);
    return record(got == expected);
}

static bool check_bool(const char *what, bool got, bool expected)
{
    if (got != expected)
        printf("  FAIL %s: got %s, expected %s\n", what,
               got ? "true" : "false", expected ? "true" : "false");
    return record(got == expected);
}

static void print_array(const int a[], int n)
{
    printf("[");
    for (int i = 0; i < n; i++)
        printf(i ? ", %d" : "%d", a[i]);
    printf("]");
}

static bool check_array(const char *what, const int got[], int got_n,
                 const int expected[], int expected_n)
{
    bool ok = (got_n == expected_n);
    for (int i = 0; ok && i < got_n; i++)
        if (got[i] != expected[i])
            ok = false;
    if (!ok) {
        printf("  FAIL %s:\n    got      ", what);
        if (got_n < 0 || got_n > 100000)
            printf("(count %d)", got_n);
        else
            print_array(got, got_n);
        printf("\n    expected ");
        print_array(expected, expected_n);
        printf("\n");
    }
    return record(ok);
}

static void insert_all(bst *t, const int keys[], int n)
{
    for (int i = 0; i < n; i++)
        bst_insert(t, keys[i]);
}

typedef int (*traversal_fn)(const bst *, int[]);

static bool check_traversal(const char *what, traversal_fn f, const bst *t,
                            const int expected[], int n)
{
    /* Size the buffer generously so a wrong count cannot overflow it. */
    int cap = n + 1000;
    int *buf = calloc((size_t)cap, sizeof *buf);
    int got = f(t, buf);
    if (got > cap)
        got = cap;
    bool ok = check_array(what, buf, got, expected, n);
    free(buf);
    return ok;
}

static bool check_preorder(const bst *t, const int expected[], int n)
{
    return check_traversal("preorder", bst_preorder, t, expected, n);
}

static bool check_inorder(const bst *t, const int expected[], int n)
{
    return check_traversal("inorder", bst_inorder, t, expected, n);
}

static bool check_postorder(const bst *t, const int expected[], int n)
{
    return check_traversal("postorder", bst_postorder, t, expected, n);
}

#define TIMEOUT_SECONDS 5

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

/* Follow a path of 'L'/'R' steps from the root. Returns NULL if the path
 * leaves the tree. Never dereferences NULL. */
static bst_node *node_at(const bst *t, const char *path)
{
    bst_node *x = t->root;
    for (const char *p = path; *p != '\0' && x != NULL; p++)
        x = (*p == 'L') ? x->left : x->right;
    return x;
}

static bool check_key_at(const bst *t, const char *path, int expected)
{
    char what[64];
    snprintf(what, sizeof what, "key at path \"%s\" from root", path);
    bst_node *x = node_at(t, path);
    if (x == NULL) {
        printf("  FAIL %s: no node there (expected %d)\n", what, expected);
        tu_checks++;
        tu_failures++;
        return false;
    }
    return check_int(what, x->key, expected);
}

static bool check_null_at(const bst *t, const char *path)
{
    char what[64];
    snprintf(what, sizeof what, "path \"%s\" from root is NULL", path);
    return check_bool(what, node_at(t, path) == NULL, true);
}

/*
 * Walk the whole tree and check:
 *   - t->root->parent == NULL
 *   - every child's parent pointer points back to its parent
 *   - BST order: left subtree keys <= node key <= right subtree keys
 *   - the tree has expected_n nodes (use -1 to skip this check)
 * Stops early on a cycle. Compares pointers only, so it never follows a
 * stale parent pointer into freed memory.
 */
#define MAX_WALK 200000
static int walk_count;

static bool check_subtree(const bst_node *x, long lo, long hi)
{
    if (x == NULL)
        return true;
    if (++walk_count > MAX_WALK) {
        printf("  FAIL structure: more than %d nodes (cycle?)\n", MAX_WALK);
        return false;
    }
    if (x->key < lo || x->key > hi) {
        printf("  FAIL structure: key %d is on the wrong side of an "
               "ancestor (BST order broken)\n", x->key);
        return false;
    }
    if (x->left != NULL && x->left->parent != x) {
        printf("  FAIL parent pointer: node %d is the left child of %d, "
               "but its parent pointer does not point to %d\n",
               x->left->key, x->key, x->key);
        return false;
    }
    if (x->right != NULL && x->right->parent != x) {
        printf("  FAIL parent pointer: node %d is the right child of %d, "
               "but its parent pointer does not point to %d\n",
               x->right->key, x->key, x->key);
        return false;
    }
    return check_subtree(x->left, lo, x->key) &&
           check_subtree(x->right, x->key, hi);
}

static bool check_structure(const bst *t, int expected_n)
{
    tu_checks++;
    walk_count = 0;
    if (t->root != NULL && t->root->parent != NULL) {
        printf("  FAIL parent pointer: root %d has a non-NULL parent\n",
               t->root->key);
        tu_failures++;
        return false;
    }
    if (!check_subtree(t->root, -2147483648L, 2147483647L)) {
        tu_failures++;
        return false;
    }
    if (expected_n >= 0 && walk_count != expected_n) {
        printf("  FAIL structure: tree has %d nodes, expected %d\n",
               walk_count, expected_n);
        tu_failures++;
        return false;
    }
    return true;
}

static bst *make_tree(const int keys[], int n)
{
    bst *t = bst_create();
    insert_all(t, keys, n);
    return t;
}

/* Deterministic pseudo-random numbers (same on every machine). */
static unsigned long rng_state;
static void rng_seed(unsigned long s) { rng_state = s; }
static int rng_below(int n)
{
    rng_state = rng_state * 6364136223846793005UL + 1442695040888963407UL;
    return (int)((rng_state >> 33) % (unsigned long)n);
}

/* The standard trees used below. Draw them before reading the tests. */
static const int TREE_A[] = {8, 2, 12, 1, 3, 10, 15};    /* perfect, height 2 */
static const int TREE_B[] = {5, 4, 7, 2, 6, 3};
static const int TREE_C[] = {5, 4, 1, 9, 6, 2, 0};
static const int TREE_D[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};
static const int TREE_E[] = {4, 2, 6, 1, 3, 5, 7};

/* ------------------------------------------------------------------ */
/* Insert, search, successor, delete, and the empty-tree rules          */
/* ------------------------------------------------------------------ */

static bool test_insert_shape(void)
{
    bst *t = bst_create();
    bst_insert(t, 5);
    const int one[] = {5};
    REQUIRE(check_preorder(t, one, 1));
    bst_destroy(t);

    t = make_tree(TREE_A, LEN(TREE_A));
    const int pa[] = {8, 2, 1, 3, 12, 10, 15};
    REQUIRE(check_preorder(t, pa, LEN(pa)));
    REQUIRE(check_key_at(t, "", 8));
    REQUIRE(check_key_at(t, "L", 2));
    REQUIRE(check_key_at(t, "LR", 3));
    REQUIRE(check_key_at(t, "RL", 10));
    REQUIRE(check_null_at(t, "LRL"));
    REQUIRE(check_null_at(t, "LRR"));
    bst_destroy(t);

    t = make_tree(TREE_B, LEN(TREE_B));
    const int pb[] = {5, 4, 2, 3, 7, 6};
    REQUIRE(check_preorder(t, pb, LEN(pb)));
    REQUIRE(check_key_at(t, "LLR", 3));
    bst_destroy(t);
    return true;
}

static bool test_contains_find(void)
{
    bst *t = make_tree(TREE_E, LEN(TREE_E));
    REQUIRE(check_bool("contains(3)", bst_contains(t, 3), true));
    REQUIRE(check_bool("contains(7)", bst_contains(t, 7), true));
    REQUIRE(check_bool("contains(4) (the root)", bst_contains(t, 4), true));
    REQUIRE(check_bool("contains(25)", bst_contains(t, 25), false));
    REQUIRE(check_bool("contains(8)", bst_contains(t, 8), false));
    REQUIRE(check_bool("contains(0)", bst_contains(t, 0), false));
    bst_node *x = bst_find(t, 6);
    REQUIRE(check_bool("find(6) is not NULL", x != NULL, true));
    REQUIRE(check_int("find(6)->key", x->key, 6));
    REQUIRE(check_bool("find(6) is root->right", x == node_at(t, "R"), true));
    REQUIRE(check_bool("find(8) is NULL", bst_find(t, 8) == NULL, true));
    bst_destroy(t);
    return true;
}

static bool test_min_max(void)
{
    int v = -999;
    bst *t = make_tree(TREE_E, LEN(TREE_E));
    REQUIRE(check_bool("min returns true", bst_min(t, &v), true));
    REQUIRE(check_int("min", v, 1));
    REQUIRE(check_bool("max returns true", bst_max(t, &v), true));
    REQUIRE(check_int("max", v, 7));
    bst_destroy(t);

    t = make_tree(TREE_D, LEN(TREE_D));
    REQUIRE(check_bool("min returns true", bst_min(t, &v), true));
    REQUIRE(check_int("min of tree D", v, 1));
    REQUIRE(check_bool("max returns true", bst_max(t, &v), true));
    REQUIRE(check_int("max of tree D", v, 14));
    bst_destroy(t);

    t = bst_create();
    bst_insert(t, 5);
    REQUIRE(check_bool("min of one node returns true", bst_min(t, &v), true));
    REQUIRE(check_int("min of one node", v, 5));
    REQUIRE(check_bool("max of one node returns true", bst_max(t, &v), true));
    REQUIRE(check_int("max of one node", v, 5));
    bst_destroy(t);
    return true;
}

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static bool test_inorder_sorted(void)
{
    bst *t = make_tree(TREE_D, LEN(TREE_D));
    const int id[] = {1, 3, 4, 6, 7, 8, 10, 13, 14};
    REQUIRE(check_inorder(t, id, LEN(id)));
    bst_destroy(t);

    t = make_tree(TREE_C, LEN(TREE_C));
    const int ic[] = {0, 1, 2, 4, 5, 6, 9};
    REQUIRE(check_inorder(t, ic, LEN(ic)));
    bst_destroy(t);

    /* 200 distinct keys in a scrambled order */
    enum { N = 200 };
    int keys[N];
    for (int i = 0; i < N; i++)
        keys[i] = (i * 73) % N * 5 - 300;
    t = make_tree(keys, N);
    qsort(keys, N, sizeof keys[0], cmp_int);
    REQUIRE(check_inorder(t, keys, N));
    bst_destroy(t);
    return true;
}

static bool test_successor(void)
{
    bst *t = make_tree(TREE_D, LEN(TREE_D));
    /* key, expected successor */
    const int cases[][2] = {
        {8, 10}, {10, 13}, {7, 8}, {13, 14}, {1, 3}, {4, 6}, {6, 7}, {3, 4},
    };
    for (int i = 0; i < LEN(cases); i++) {
        char what[64];
        int v = -999;
        snprintf(what, sizeof what, "successor(%d) returns true", cases[i][0]);
        REQUIRE(check_bool(what, bst_successor(t, cases[i][0], &v), true));
        snprintf(what, sizeof what, "successor(%d)", cases[i][0]);
        REQUIRE(check_int(what, v, cases[i][1]));
    }
    int v = -999;
    REQUIRE(check_bool("successor(14) of the max returns false",
                       bst_successor(t, 14, &v), false));
    REQUIRE(check_int("out-parameter untouched when false", v, -999));
    bst_destroy(t);

    t = bst_create();
    bst_insert(t, 5);
    REQUIRE(check_bool("successor(5) in a one-node tree returns false",
                       bst_successor(t, 5, &v), false));
    bst_destroy(t);
    return true;
}

static bool test_delete_leaf(void)
{
    bst *t = make_tree(TREE_C, LEN(TREE_C));
    REQUIRE(check_bool("delete(6) returns true", bst_delete(t, 6), true));
    const int in1[] = {0, 1, 2, 4, 5, 9};
    const int pre1[] = {5, 4, 1, 0, 2, 9};
    REQUIRE(check_inorder(t, in1, LEN(in1)));
    REQUIRE(check_preorder(t, pre1, LEN(pre1)));
    REQUIRE(check_null_at(t, "RL"));
    REQUIRE(check_bool("contains(6) after delete", bst_contains(t, 6), false));

    REQUIRE(check_bool("delete(0) returns true", bst_delete(t, 0), true));
    const int pre2[] = {5, 4, 1, 2, 9};
    REQUIRE(check_preorder(t, pre2, LEN(pre2)));
    bst_destroy(t);
    return true;
}

static bool test_delete_one_child(void)
{
    /* node with only a left child */
    bst *t = make_tree(TREE_C, LEN(TREE_C));
    REQUIRE(check_bool("delete(4) returns true", bst_delete(t, 4), true));
    const int pre1[] = {5, 1, 0, 2, 9, 6};
    REQUIRE(check_preorder(t, pre1, LEN(pre1)));
    REQUIRE(check_bool("delete(9) returns true", bst_delete(t, 9), true));
    const int pre2[] = {5, 1, 0, 2, 6};
    REQUIRE(check_preorder(t, pre2, LEN(pre2)));
    bst_destroy(t);

    /* node with only a right child */
    t = make_tree(TREE_B, LEN(TREE_B));
    REQUIRE(check_bool("delete(2) returns true", bst_delete(t, 2), true));
    const int pre3[] = {5, 4, 3, 7, 6};
    REQUIRE(check_preorder(t, pre3, LEN(pre3)));
    bst_destroy(t);
    return true;
}

static bool test_delete_two_children(void)
{
    /* successor is z's right child */
    bst *t = make_tree(TREE_C, LEN(TREE_C));
    REQUIRE(check_bool("delete(1) returns true", bst_delete(t, 1), true));
    const int pre1[] = {5, 4, 2, 0, 9, 6};
    REQUIRE(check_preorder(t, pre1, LEN(pre1)));
    REQUIRE(check_bool("contains(1) after delete", bst_contains(t, 1), false));
    bst_destroy(t);

    /* successor (4) is deeper in z's right subtree, not z's right child */
    t = make_tree(TREE_D, LEN(TREE_D));
    REQUIRE(check_bool("delete(3) returns true", bst_delete(t, 3), true));
    const int pre2[] = {8, 4, 1, 6, 7, 10, 14, 13};
    REQUIRE(check_preorder(t, pre2, LEN(pre2)));
    bst_destroy(t);

    /* successor (7) is z's right child, and z's left child has no kids */
    t = make_tree(TREE_D, LEN(TREE_D));
    REQUIRE(check_bool("delete(6) returns true", bst_delete(t, 6), true));
    const int pre3[] = {8, 3, 1, 7, 4, 10, 14, 13};
    REQUIRE(check_preorder(t, pre3, LEN(pre3)));
    bst_destroy(t);
    return true;
}

static bool test_delete_root(void)
{
    bst *t = make_tree(TREE_C, LEN(TREE_C));
    REQUIRE(check_bool("delete(5) (root, two children) returns true",
                       bst_delete(t, 5), true));
    const int pre1[] = {6, 4, 1, 0, 2, 9};
    REQUIRE(check_preorder(t, pre1, LEN(pre1)));
    REQUIRE(check_bool("delete(6) (root, successor is right child)",
                       bst_delete(t, 6), true));
    const int pre2[] = {9, 4, 1, 0, 2};
    REQUIRE(check_preorder(t, pre2, LEN(pre2)));
    REQUIRE(check_bool("delete(9) (root, left child only)",
                       bst_delete(t, 9), true));
    const int pre3[] = {4, 1, 0, 2};
    REQUIRE(check_preorder(t, pre3, LEN(pre3)));
    REQUIRE(check_key_at(t, "", 4));
    bst_destroy(t);

    t = make_tree((const int[]){1, 2, 3}, 3);
    REQUIRE(check_bool("delete(1) (root, right child only)",
                       bst_delete(t, 1), true));
    const int pre4[] = {2, 3};
    REQUIRE(check_preorder(t, pre4, LEN(pre4)));
    bst_destroy(t);

    t = bst_create();
    bst_insert(t, 7);
    REQUIRE(check_bool("delete(7) (only node)", bst_delete(t, 7), true));
    REQUIRE(check_bool("root is NULL after deleting the only node",
                       t->root == NULL, true));
    bst_destroy(t);
    return true;
}

static bool test_parents_after_delete(void)
{
    bst *t = make_tree(TREE_C, LEN(TREE_C));
    bst_delete(t, 4);                       /* one child (left) */
    REQUIRE(check_structure(t, 6));
    bst_delete(t, 5);                       /* root, two children */
    REQUIRE(check_structure(t, 5));
    bst_destroy(t);

    t = make_tree(TREE_B, LEN(TREE_B));
    bst_delete(t, 2);                       /* one child (right) */
    REQUIRE(check_structure(t, 5));
    bst_destroy(t);

    t = make_tree(TREE_D, LEN(TREE_D));
    bst_delete(t, 3);                       /* two children, deep successor */
    REQUIRE(check_structure(t, 8));
    bst_delete(t, 10);                      /* one child (right) */
    REQUIRE(check_structure(t, 7));
    bst_delete(t, 8);                       /* root */
    REQUIRE(check_structure(t, 6));
    bst_destroy(t);

    t = make_tree((const int[]){1, 2, 3}, 3);
    bst_delete(t, 1);                       /* root with right child only */
    REQUIRE(check_structure(t, 2));
    bst_destroy(t);
    return true;
}

/* An empty traversal must write nothing: every cell of buf must still
 * hold the sentinel -1. (A failed check ends the test, so buf is still
 * all -1 before each traversal.) */
static bool check_untouched(const char *what, const int buf[], int n)
{
    for (int i = 0; i < n; i++) {
        if (buf[i] != -1) {
            printf("  FAIL %s on an empty tree wrote out[%d] = %d; "
                   "it must write nothing\n", what, i, buf[i]);
            return record(false);
        }
    }
    return record(true);
}

static bool test_empty_and_missing(void)
{
    int v = -999;
    int buf[4] = {-1, -1, -1, -1};
    bst *t = bst_create();
    REQUIRE(check_bool("empty: min returns false", bst_min(t, &v), false));
    REQUIRE(check_bool("empty: max returns false", bst_max(t, &v), false));
    REQUIRE(check_int("empty: out-parameter untouched", v, -999));
    REQUIRE(check_bool("empty: successor(4) returns false",
                       bst_successor(t, 4, &v), false));
    REQUIRE(check_bool("empty: delete(5) returns false", bst_delete(t, 5), false));
    REQUIRE(check_bool("empty: contains(5)", bst_contains(t, 5), false));
    REQUIRE(check_bool("empty: find(5) is NULL", bst_find(t, 5) == NULL, true));
    REQUIRE(check_int("empty: size", bst_size(t), 0));
    REQUIRE(check_int("empty: height", bst_height(t), -1));
    REQUIRE(check_int("empty: preorder count", bst_preorder(t, buf), 0));
    REQUIRE(check_untouched("empty: preorder", buf, LEN(buf)));
    REQUIRE(check_int("empty: inorder count", bst_inorder(t, buf), 0));
    REQUIRE(check_untouched("empty: inorder", buf, LEN(buf)));
    REQUIRE(check_int("empty: postorder count", bst_postorder(t, buf), 0));
    REQUIRE(check_untouched("empty: postorder", buf, LEN(buf)));
    REQUIRE(check_bool("empty: root still NULL", t->root == NULL, true));
    bst_destroy(t);

    t = make_tree(TREE_D, LEN(TREE_D));
    const int pre[] = {8, 3, 1, 6, 4, 7, 10, 14, 13};
    REQUIRE(check_bool("delete(5) of a missing key returns false",
                       bst_delete(t, 5), false));
    REQUIRE(check_preorder(t, pre, LEN(pre)));
    REQUIRE(check_int("size after failed delete", bst_size(t), 9));
    REQUIRE(check_bool("successor(5) of a missing key returns false",
                       bst_successor(t, 5, &v), false));
    REQUIRE(check_bool("successor(100) of a missing key returns false",
                       bst_successor(t, 100, &v), false));
    REQUIRE(check_int("out-parameter untouched", v, -999));
    REQUIRE(check_bool("delete(7) returns true", bst_delete(t, 7), true));
    REQUIRE(check_bool("delete(7) again returns false", bst_delete(t, 7), false));
    bst_destroy(t);
    bst_destroy(NULL);                      /* must be a no-op */
    return true;
}

/* ------------------------------------------------------------------ */
/* Other tests                                                          */
/* ------------------------------------------------------------------ */

static bool insert_parent_pointers(void)
{
    bst *t = make_tree(TREE_A, LEN(TREE_A));
    REQUIRE(check_bool("root->parent is NULL", t->root->parent == NULL, true));
    REQUIRE(check_bool("15's parent is 12",
                       node_at(t, "RR")->parent == node_at(t, "R"), true));
    REQUIRE(check_bool("2's parent is 8",
                       node_at(t, "L")->parent == t->root, true));
    REQUIRE(check_structure(t, 7));
    bst_destroy(t);

    t = make_tree(TREE_D, LEN(TREE_D));
    REQUIRE(check_structure(t, 9));
    bst_destroy(t);
    return true;
}

static bool postorder_test(void)
{
    bst *t = make_tree(TREE_C, LEN(TREE_C));
    const int pc[] = {0, 2, 1, 4, 6, 9, 5};
    REQUIRE(check_postorder(t, pc, LEN(pc)));
    bst_destroy(t);

    t = make_tree(TREE_D, LEN(TREE_D));
    const int pd[] = {1, 4, 7, 6, 3, 13, 14, 10, 8};
    REQUIRE(check_postorder(t, pd, LEN(pd)));
    bst_destroy(t);

    t = bst_create();
    bst_insert(t, 5);
    const int one[] = {5};
    REQUIRE(check_postorder(t, one, 1));
    bst_destroy(t);
    return true;
}

static bool size_test(void)
{
    bst *t = bst_create();
    REQUIRE(check_int("size of empty tree", bst_size(t), 0));
    bst_insert(t, 5);
    REQUIRE(check_int("size of one node", bst_size(t), 1));
    bst_destroy(t);

    t = make_tree(TREE_C, LEN(TREE_C));
    REQUIRE(check_int("size of tree C", bst_size(t), 7));
    bst_delete(t, 1);
    REQUIRE(check_int("size after one delete", bst_size(t), 6));
    bst_delete(t, 42);
    REQUIRE(check_int("size after deleting a missing key", bst_size(t), 6));
    bst_destroy(t);
    return true;
}

static bool height_test(void)
{
    bst *t = bst_create();
    bst_insert(t, 5);
    REQUIRE(check_int("height of one node", bst_height(t), 0));
    bst_destroy(t);

    t = make_tree(TREE_A, LEN(TREE_A));
    REQUIRE(check_int("height of tree A (perfect, 7 nodes)", bst_height(t), 2));
    bst_destroy(t);

    t = make_tree(TREE_C, LEN(TREE_C));
    REQUIRE(check_int("height of tree C", bst_height(t), 3));
    bst_delete(t, 0);
    REQUIRE(check_int("height of tree C after delete(0)", bst_height(t), 3));
    bst_delete(t, 2);
    REQUIRE(check_int("height after delete(2)", bst_height(t), 2));
    bst_destroy(t);

    t = make_tree(TREE_D, LEN(TREE_D));
    REQUIRE(check_int("height of tree D", bst_height(t), 3));
    bst_destroy(t);
    return true;
}

static bool skewed_tree(void)
{
    enum { N = 10 };
    int up[N], down[N];
    for (int i = 0; i < N; i++) {
        up[i] = i;
        down[i] = N - 1 - i;
    }

    bst *t = make_tree(up, N);
    REQUIRE(check_int("height after inserting 0..9 in order", bst_height(t), N - 1));
    REQUIRE(check_preorder(t, up, N));
    REQUIRE(check_null_at(t, "L"));
    REQUIRE(check_key_at(t, "RRRRRRRRR", 9));
    REQUIRE(check_structure(t, N));
    bst_destroy(t);

    t = make_tree(down, N);
    REQUIRE(check_int("height after inserting 9..0 in order", bst_height(t), N - 1));
    REQUIRE(check_preorder(t, down, N));
    REQUIRE(check_inorder(t, up, N));
    bst_destroy(t);

    /* Same keys 1..7, different insertion order, very different shape. */
    t = make_tree(TREE_E, LEN(TREE_E));
    REQUIRE(check_int("height after inserting 4 2 6 1 3 5 7", bst_height(t), 2));
    bst_destroy(t);

    /* A long chain: 3000 keys in increasing order. */
    enum { BIG = 3000 };
    t = bst_create();
    for (int i = 0; i < BIG; i++)
        bst_insert(t, i);
    REQUIRE(check_int("height of a 3000-node chain", bst_height(t), BIG - 1));
    REQUIRE(check_int("size of a 3000-node chain", bst_size(t), BIG));
    bst_destroy(t);
    return true;
}

static bool duplicates_go_right(void)
{
    bst *t = make_tree((const int[]){5, 3, 5, 7, 5}, 5);
    const int pre[] = {5, 3, 5, 7, 5};
    const int in[] = {3, 5, 5, 5, 7};
    REQUIRE(check_preorder(t, pre, LEN(pre)));
    REQUIRE(check_inorder(t, in, LEN(in)));
    REQUIRE(check_key_at(t, "R", 5));
    REQUIRE(check_key_at(t, "RRL", 5));
    REQUIRE(check_bool("find(5) is the root", bst_find(t, 5) == t->root, true));
    int v = -999;
    REQUIRE(check_bool("successor(5) returns true", bst_successor(t, 5, &v), true));
    REQUIRE(check_int("successor(5) is the next copy of 5", v, 5));

    REQUIRE(check_bool("delete(5) #1", bst_delete(t, 5), true));
    const int p1[] = {5, 3, 7, 5};
    REQUIRE(check_preorder(t, p1, LEN(p1)));
    REQUIRE(check_bool("delete(5) #2", bst_delete(t, 5), true));
    const int p2[] = {5, 3, 7};
    REQUIRE(check_preorder(t, p2, LEN(p2)));
    REQUIRE(check_bool("delete(5) #3", bst_delete(t, 5), true));
    const int p3[] = {7, 3};
    REQUIRE(check_preorder(t, p3, LEN(p3)));
    REQUIRE(check_bool("delete(5) #4 (none left)", bst_delete(t, 5), false));
    REQUIRE(check_structure(t, 2));
    bst_destroy(t);

    /* A duplicated maximum: bst_find gives the root copy, and its
     * successor is the other copy, so successor(5) succeeds. */
    t = make_tree((const int[]){5, 5}, 2);
    v = -999;
    REQUIRE(check_bool("tree 5 5: successor(5) returns true",
                       bst_successor(t, 5, &v), true));
    REQUIRE(check_int("tree 5 5: successor(5) is the other copy", v, 5));
    REQUIRE(check_bool("tree 5 5: delete(5)", bst_delete(t, 5), true));
    v = -999;
    REQUIRE(check_bool("one 5 left: successor(5) returns false",
                       bst_successor(t, 5, &v), false));
    REQUIRE(check_int("out-parameter untouched", v, -999));
    bst_destroy(t);
    return true;
}

static bool transplant_test(void)
{
    bst *t = make_tree(TREE_A, LEN(TREE_A));
    bst_node *n8 = t->root, *n2 = node_at(t, "L"), *n1 = node_at(t, "LL");
    bst_node *n12 = node_at(t, "R"), *n10 = node_at(t, "RL");
    bst_node *n15 = node_at(t, "RR");

    /* replace a right child (12) by its own right child (15) */
    bst_transplant(t, n12, n15);
    REQUIRE(check_bool("8->right is now 15", n8->right == n15, true));
    REQUIRE(check_bool("15->parent is now 8", n15->parent == n8, true));

    /* replace the root (8) by its left child (2) */
    bst_transplant(t, n8, n2);
    REQUIRE(check_bool("root is now 2", t->root == n2, true));
    REQUIRE(check_bool("2->parent is now NULL", n2->parent == NULL, true));

    /* replace a left child (1) by NULL */
    bst_transplant(t, n1, NULL);
    REQUIRE(check_bool("2->left is now NULL", n2->left == NULL, true));
    const int pre[] = {2, 3};
    REQUIRE(check_preorder(t, pre, LEN(pre)));

    /* Nodes 8, 12, 10, 15, 1 are no longer in the tree: free them here. */
    free(n8);
    free(n12);
    free(n10);
    free(n15);
    free(n1);
    bst_destroy(t);
    return true;
}

static bool random_stress(void)
{
    enum { RANGE = 300, OPS = 3000 };
    bool in_model[RANGE] = {false};
    int n = 0;
    bst *t = bst_create();
    rng_seed(313);
    for (int op = 1; op <= OPS; op++) {
        int k = rng_below(RANGE);
        char what[80];
        if (rng_below(5) < 3) {
            if (!in_model[k]) {             /* keep keys distinct */
                bst_insert(t, k);
                in_model[k] = true;
                n++;
            }
        } else {
            snprintf(what, sizeof what, "op %d: delete(%d) return value", op, k);
            REQUIRE(check_bool(what, bst_delete(t, k), in_model[k]));
            if (in_model[k]) {
                in_model[k] = false;
                n--;
            }
        }
        REQUIRE(check_structure(t, n));
        if (op % 100 == 0) {
            int expected[RANGE], m = 0;
            for (int i = 0; i < RANGE; i++)
                if (in_model[i])
                    expected[m++] = i;
            REQUIRE(check_inorder(t, expected, m));
            for (int j = 0; j < 20; j++) {
                int q = rng_below(RANGE), v = -1, want = -1;
                bool has = false;
                if (in_model[q])
                    for (int i = q + 1; i < RANGE && !has; i++)
                        if (in_model[i]) {
                            has = true;
                            want = i;
                        }
                snprintf(what, sizeof what, "op %d: successor(%d) return value", op, q);
                REQUIRE(check_bool(what, bst_successor(t, q, &v), has));
                if (has) {
                    snprintf(what, sizeof what, "op %d: successor(%d)", op, q);
                    REQUIRE(check_int(what, v, want));
                }
            }
        }
    }
    bst_destroy(t);
    return true;
}

static bool delete_until_empty(void)
{
    bst *t = make_tree(TREE_D, LEN(TREE_D));
    const int order[] = {8, 13, 3, 14, 1, 6, 10, 4, 7};
    for (int i = 0; i < LEN(order); i++) {
        char what[48];
        snprintf(what, sizeof what, "delete(%d) returns true", order[i]);
        REQUIRE(check_bool(what, bst_delete(t, order[i]), true));
        REQUIRE(check_structure(t, LEN(order) - 1 - i));
    }
    REQUIRE(check_bool("root is NULL", t->root == NULL, true));
    REQUIRE(check_int("size", bst_size(t), 0));
    /* The tree is still usable. */
    insert_all(t, TREE_B, LEN(TREE_B));
    const int pb[] = {5, 4, 2, 3, 7, 6};
    REQUIRE(check_preorder(t, pb, LEN(pb)));
    bst_destroy(t);
    return true;
}

/* Run under valgrind by the autograder; exercises every path that frees. */
static bool leak_scenario(void)
{
    bst *t = bst_create();
    bst_destroy(t);                          /* empty tree */
    t = make_tree(TREE_D, LEN(TREE_D));
    bst_delete(t, 13);                       /* leaf */
    bst_delete(t, 10);                       /* one child */
    bst_delete(t, 3);                        /* two children */
    bst_delete(t, 8);                        /* root */
    bst_delete(t, 99);                       /* missing */
    int buf[16];
    bst_preorder(t, buf);
    bst_inorder(t, buf);
    bst_postorder(t, buf);
    bst_size(t);
    bst_height(t);
    int v;
    bst_min(t, &v);
    bst_max(t, &v);
    bst_successor(t, 4, &v);
    bst_destroy(t);
    return true;
}


/* ------------------------------------------------------------------ */
/* Test table and runner                                                */
/* ------------------------------------------------------------------ */

typedef struct {
    const char *name;
    bool (*fn)(void);
} test_case;

static const test_case TESTS[] = {
    {"insert_shape", test_insert_shape},
    {"contains_find", test_contains_find},
    {"min_max", test_min_max},
    {"inorder_sorted", test_inorder_sorted},
    {"successor", test_successor},
    {"delete_leaf", test_delete_leaf},
    {"delete_one_child", test_delete_one_child},
    {"delete_two_children", test_delete_two_children},
    {"delete_root", test_delete_root},
    {"parents_after_delete", test_parents_after_delete},
    {"empty_and_missing", test_empty_and_missing},
    {"insert_parent_pointers", insert_parent_pointers},
    {"postorder", postorder_test},
    {"size", size_test},
    {"height", height_test},
    {"skewed_tree", skewed_tree},
    {"duplicates_go_right", duplicates_go_right},
    {"transplant", transplant_test},
    {"random_stress", random_stress},
    {"delete_until_empty", delete_until_empty},
    /* Not a test by itself: the autograder runs it under valgrind as
     * "leak_check". Run it yourself with "make valgrind". */
    {"leak_scenario", leak_scenario},
};
#define NUM_TESTS ((int)(sizeof TESTS / sizeof TESTS[0]))

static int run_one_here(const test_case *tc)
{
    tu_checks = tu_failures = 0;
    bool ok = tc->fn();
    if (ok && tu_failures == 0) {
        printf("passed\n");
        return 0;
    }
    return 1;
}

/* Fork so that a crash or infinite loop fails only this test. */
static int run_one_isolated(const test_case *tc)
{
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        alarm(TIMEOUT_SECONDS);
        _exit(run_one_here(tc));
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        if (sig == SIGALRM)
            printf("  FAIL timed out after %d seconds (infinite loop?)\n",
                   TIMEOUT_SECONDS);
        else
            printf("  FAIL crashed with signal %d%s\n", sig,
                   sig == SIGSEGV ? " (segmentation fault)" : "");
    }
    return 2;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc == 2 && strcmp(argv[1], "--list") == 0) {
        for (int i = 0; i < NUM_TESTS - 1; i++)
            printf("%s\n", TESTS[i].name);
        return 0;
    }
    if (argc == 2) {
        for (int i = 0; i < NUM_TESTS; i++)
            if (strcmp(argv[1], TESTS[i].name) == 0)
                return run_one_here(&TESTS[i]);
        fprintf(stderr, "unknown test %s (try --list)\n", argv[1]);
        return 2;
    }

    int passed = 0;
    for (int i = 0; i < NUM_TESTS - 1; i++) {      /* skip leak_scenario */
        printf("%s:\n", TESTS[i].name);
        int rc = run_one_isolated(&TESTS[i]);
        if (rc != 0)
            printf("  FAILED\n");
        passed += (rc == 0);
    }
    printf("\nTests passed: %d/%d\n", passed, NUM_TESTS - 1);
    printf("Gradescope also runs leak_check (valgrind), your three "
           "student tests, and a check that REPORT.md is filled in.\n");
    return passed == NUM_TESTS - 1 ? 0 : 1;
}
