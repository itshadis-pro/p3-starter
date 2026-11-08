# P3 Report

Name: Hadi Almarzooq

1. In two to four sentences, explain how your `bst_delete` removes a node z that has two children. Say how you find its successor y, and which pointers change, parent pointers included, when y is not z's right child.

When deleting a node with two children, my code finds the node's successor, which is the absolute minimum of its right subtree. Instead of just copying the key, it moves the actual successor node into the deleted node's position. It uses `bst_transplant` twice: first to replace the successor with its own right child (if the successor isn't the direct right child of the deleted node), and second to transplant the successor over the deleted node, updating the left and right child pointers to adopt the deleted node's children.

2. In one or two sentences, explain why your `bst_successor` takes O(h) time on a tree of height h. Hint: which way does each step move, in the search for the key and in each of the two cases?


The `bst_successor` function only ever traces a single path through the tree: it either walks straight down the right subtree to find a minimum, or it walks straight up the parent pointers to find an ancestor. Because it never visits every node and only travels vertically along a single branch, the maximum number of steps it can possibly take is bounded by the height of the tree, $h$.

