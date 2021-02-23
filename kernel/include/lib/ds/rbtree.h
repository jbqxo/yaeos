#ifndef _LIB_DS_RBTREE_H
#define _LIB_DS_RBTREE_H

#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef int (*rbtree_cmp_fn)(const void *tree_val, const void *external_val);

enum rbtree_colour { RBTREE_BLACK = 0, RBTREE_RED = 1 };

struct rbtree_node {
        void *data;
        struct rbtree_node *left;
        struct rbtree_node *right;
        union uiptr parent;

#ifndef NDEBUG
        enum rbtree_colour colour : 1;
#endif
};

struct rbtree {
        struct rbtree_node *root;
};

void rbtree_init_tree(struct rbtree *rbt);

void rbtree_init_node(struct rbtree_node *node);

void rbtree_insert(struct rbtree *rbt, struct rbtree_node *new, rbtree_cmp_fn cmpf);

void rbtree_delete(struct rbtree *rbt, struct rbtree_node *deletee);

struct rbtree_node *rbtree_search(struct rbtree *rbt, void *value, rbtree_cmp_fn cmpf);

struct rbtree_node *rbtree_search_min(struct rbtree *rbt, void *limit, rbtree_cmp_fn cmpf);
struct rbtree_node *rbtree_search_max(struct rbtree *rbt, void *limit, rbtree_cmp_fn cmpf);

void rbtree_iter_range(struct rbtree *rbt, void *value_from, void *value_to, rbtree_cmp_fn cmpf,
                       bool (*fn)(void *elem, void *data), void *data);

#endif /* _LIB_DS_RBTREE_H */
