#ifndef _KERNEL_DS_RBTREE_H
#define _KERNEL_DS_RBTREE_H

#include "kernel/cppdefs.h"

#include "lib/assert.h"

#include <stddef.h>
#include <stdint.h>

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
	int (*cmp)(void *, void *);
};

void rbt_set_colour(struct rbtree_node *n, enum rbtree_colour c);

enum rbtree_colour rbt_get_colour(struct rbtree_node *n);

void rbt_set_parent(struct rbtree_node *node, struct rbtree_node *parent);

struct rbtree_node *rbt_get_parent(struct rbtree_node *node);

struct rbtree_node *rbt_get_grandparent(struct rbtree_node *node);

struct rbtree_node *rbt_get_sibling(struct rbtree_node *node);

struct rbtree_node *rbt_get_uncle(struct rbtree_node *node);

void rbt_replace_subtree(struct rbtree *rbt, struct rbtree_node *replacee,
			 struct rbtree_node *replacement);

void rbt_swap_nodes(struct rbtree *rbt, struct rbtree_node *x, struct rbtree_node *y);

void rbtree_init_tree(struct rbtree *rbt, int (*cmp)(void *, void *));

void rbtree_init_node(struct rbtree_node *node);

void rbt_rotate_left(struct rbtree *rbt, struct rbtree_node *old_root);

void rbt_rotate_right(struct rbtree *rbt, struct rbtree_node *old_root);

void rbt_insert_fix(struct rbtree *rbt, struct rbtree_node *new);

void rbtree_insert(struct rbtree *rbt, struct rbtree_node *new);

struct rbtree_node *rbt_find_successor(struct rbtree_node *subtree);

void rbt_delete_fix(struct rbtree *rbt, struct rbtree_node *node);

void rbtree_delete(struct rbtree *rbt, struct rbtree_node *deletee);

struct rbtree_node *rbtree_search(struct rbtree *rbt, void *value);

struct rbtree_node *rbtree_search_min(struct rbtree *rbt, void *limit);

void rbtree_iter_range(struct rbtree *rbt, void *value_from, void *value_to,
		       bool (*fn)(void *elem, void *data), void *data);

#endif // _KERNEL_DS_RBTREE_H
