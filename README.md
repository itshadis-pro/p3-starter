# P3: Binary search tree with parent pointers

CS 313, Fall 2026. Posted Tue 10/27.

- **First step (in lab Tue 10/27, or on your own):** fill in `bst_insert` and `bst_destroy`. Submit `bst.c` to Gradescope, and check that the `insert_parent_pointers` test passes. This makes sure your setup and submission work well before the deadline.
- **Due:** Fri 11/6, 11:59pm. Free extension to Sat 11/7, 11:59pm.
- **Language:** C. You may use AI tools as a tutor when you get stuck, but you need to understand your code: you will explain it in `REPORT.md`, and exams include BST traces and questions about code like this.
- **Set up first:** follow `SETUP.md`. You did this for P1 and P2; if you skipped it, do it now. Everything below assumes that setup.
- **Why it matters:** Midterm 2 (Wed 11/18) has a bug hunt on BST code taken from this assignment.

## The idea

You build the BST from CLRS Chapter 12 in C: insert, search, min and max, successor, delete, and the three traversals. Each node stores a pointer to its parent. Parent pointers let successor walk up the tree without a stack. They are also the pointers that delete most often gets wrong.

Follow the CLRS pseudocode. It maps almost line for line to C: NIL becomes `NULL`, `T.root` becomes `t->root`, and `x.left` becomes `x->left`.

## Files

| File | What to do |
|---|---|
| `bst.h` | The API and the rules. Read it. Do not edit it. |
| `bst.c` | Fill in every TODO. **Submit.** |
| `test_student.c` | Write three tests (below). **Submit.** |
| `REPORT.md` | Answer two short questions about your code. **Submit.** |
| `test_bst.c` | The autograder's tests, for local runs. Do not edit. |
| `check.h` | Tiny test framework for `test_student.c`. Do not edit. |
| `SETUP.md` | How to set up gcc, make, and valgrind. Same as P1 and P2. |
| `.devcontainer/` | Lets GitHub Codespaces set up everything for you (Option C in `SETUP.md`). |
| `Makefile` | `make`, `make check`, `make test`, `make asan`, `make valgrind`, `make clean` |

## The API

A `bst_node` has `key`, `left`, `right`, and `parent`. A `bst` holds `root`, which is `NULL` for an empty tree.

- `bst_create()`: new empty tree (provided).
- `bst_destroy(t)`: free every node and the tree.
- `bst_insert(t, key)`: add key as a new leaf.
- `bst_find(t, key)`: the node holding key, or `NULL`.
- `bst_contains(t, key)`: true if key is in the tree.
- `bst_min(t, &out)`, `bst_max(t, &out)`: smallest and largest key.
- `bst_successor(t, key, &out)`: the key that follows the node `bst_find(t, key)` returns, in an inorder walk.
- `bst_transplant(t, u, v)`: CLRS TRANSPLANT; used by delete.
- `bst_delete(t, key)`: remove one copy of key (CLRS TREE-DELETE).
- `bst_preorder(t, out)`, `bst_inorder(t, out)`, `bst_postorder(t, out)`: write the keys into the array `out` and return how many.
- `bst_size(t)`: number of nodes.
- `bst_height(t)`: edges on the longest root-to-leaf path.
- `bst_print(t)`: print the tree sideways for debugging (provided).

## The rules

- **Failure returns false.** `bst_min`, `bst_max`, `bst_successor`, and `bst_delete` return a `bool`. On failure they return `false` and leave `*out` and the tree unchanged. They never print, exit, or crash.
- **Failures:** min or max of an empty tree; successor of a missing key, or of a key whose node (the one `bst_find` returns) is the last node in an inorder walk; delete of a missing key.
- **Duplicates go right**, as in CLRS. At node x, go left if `key < x->key`, else right. `bst_find` returns the copy closest to the root. `bst_delete` removes that copy. `bst_successor` starts from that copy, so the successor of a duplicated key is another copy of the same key. This holds for the maximum too: in a tree of 5 and 5, the successor of 5 is 5.
- **Delete of a node with two children** replaces it with its successor, the minimum of its right subtree. Do not use the predecessor. The tests check the exact shape.
- **Height:** one node has height 0; an empty tree has height -1.
- **Traversals** return 0 on an empty tree. The caller's array must have room for `bst_size(t)` ints.

## Recursion on trees

CS 212 did not teach recursion, so here is one worked example. A tree is either empty, or a root with a left subtree and a right subtree. Each subtree is itself a tree. So many tree functions follow one pattern: handle the empty tree, then combine the answers for the two subtrees.

```c
/* Number of nodes in the subtree rooted at x. */
static int size_rec(const bst_node *x) {
    if (x == NULL) {        /* empty tree: the base case */
        return 0;
    }
    return 1 + size_rec(x->left) + size_rec(x->right);
}

int bst_size(const bst *t) {
    return size_rec(t->root);
}
```

Trace it on the tree 5 (left 2, right 8). `size_rec(5)` calls `size_rec(2)`. That node's children are both `NULL`, so it returns 1 + 0 + 0 = 1. Then `size_rec(8)` also returns 1. So `size_rec(5)` returns 1 + 1 + 1 = 3.

`bst_height` and `bst_destroy` follow the same pattern. For the traversals, you may write recursive or iterative code. A recursive traversal helper can take the next free index in `out` and return the next free index after it.

## Your three tests

In `test_student.c`, write three tests. Each one inserts (and maybe deletes) keys, then checks a traversal with `CHECK_PREORDER(t, ...)` or `CHECK_POSTORDER(t, ...)`. Draw the tree by hand first, and put the drawing in a comment. The keys must be plain numbers, such as `CHECK_PREORDER(t, 5, 2, 1, 8, 9)`.

The autograder runs your tests against the instructor's `bst.c`. A test passes if all its checks pass there and it checks a preorder or postorder of at least 5 keys. So a wrong hand trace fails. Run `make test` to try them on your own code.

## The report

`REPORT.md` asks two short questions: how your `bst_delete` removes a node with two children, and why your `bst_successor` takes O(h) time. Answer in your own words, about your own code. Replace every `____`.

## Test locally

- `make check` runs the autograder's tests on your `bst.c`. Each test has a 5-second limit. A failing test prints what it expected. Unless every test passes, `make` ends with an error line such as `make: *** [check] Error 1`.
- `./test_bst delete_root` runs one test. Use this form under a debugger. `./test_bst --list` lists the tests.
- `make test` runs your three tests.
- `make asan` runs the tests with AddressSanitizer, which reports memory errors with a line number.
- `make valgrind` checks for leaks, if you have valgrind.

Call `bst_print(t)` anywhere to see the tree. It prints the root at the left edge, with right subtrees above.

## Grading

P3 is worth 10 points, the same as each of the other programming assignments. Submit `bst.c`, `test_student.c`, and `REPORT.md`. The autograder uses its own copies of `bst.h`, `test_bst.c`, and `check.h`, so do not change them.

- **8 points from the autograder.** The points are split evenly across all the tests, and each passing test earns its share. The tests check insert shape, contains and find, min and max, sorted inorder, successor, delete of a leaf, of a node with one child, of a node with two children, and of the root, parent pointers after insert and after delete, the empty-tree and missing-key rules, postorder, size, height, a skewed tree, duplicates, transplant on its own, 3000 random inserts and deletes, deleting every key, no memory leaks (under `valgrind`), your three hand-traced tests, and that `REPORT.md` is filled in. Gradescope shows every test and its message, and you may resubmit as often as you like.
- **2 points graded by the course staff:** readable code (clear names, consistent indentation, comments where the logic is not obvious, no leftover debugging output), the required methods (`bst_delete` follows CLRS TREE-DELETE: it uses `bst_transplant` and moves the successor node itself into the deleted node's place, instead of copying keys between nodes; `bst_successor` uses the two CLRS cases, not a traversal; and no function copies the keys into an array to rebuild or search the tree), and correct explanations in `REPORT.md` of how your delete and successor work.

Working code is expected: if you are stuck, office hours, the Canvas discussion, and AI tools can all help you get there.

The autograder runs your code with AddressSanitizer. `heap-use-after-free` usually means a parent pointer still points at a node you freed.

## Check your understanding

Do these on paper before you write the code. Each is the kind of question Midterm 2 asks.

1. Insert 8, 3, 10, 1, 6, 14, 4, 7, 13 into an empty tree. Draw it. Write its preorder and postorder.
2. In that tree, find the successor of 7 and of 13 by following CLRS TREE-SUCCESSOR. Which case does each one use?
3. Delete 3 from that tree. Which node replaces it? Which parent pointers change?
4. Insert 1, 2, 3, 4, 5 in that order. What is the height? What order of the same keys gives height 2?
5. In TRANSPLANT, what goes wrong if you forget `v->parent = u->parent`? Which later operation would notice?

## Tips

- Draw the pointers before and after each delete case. The hardest case changes up to eight pointers.
- Write `bst_transplant` before `bst_delete`, and test it with `./test_bst transplant`.
- Check for `NULL` before you follow `x->left`, `x->right`, or `x->parent`.
- Run `make asan` before you submit, and fix every warning from `gcc -Wall`.
- Run `make valgrind` before you submit, if you have valgrind. You want "All heap blocks were freed" and "ERROR SUMMARY: 0 errors". (valgrind does not run on recent Macs; see `SETUP.md`. The autograder checks for leaks either way.)
