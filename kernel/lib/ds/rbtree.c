#include "lib/ds/rbtree.h"
#include "lib/cppdefs.h"

union nodeptr {
        struct rbtree_node *ptr;
        uintptr_t num;
};

static void rbt_set_colour(struct rbtree_node *n, enum rbtree_colour c)
{
        kassert(n);

#ifndef NDEBUG
        n->colour = c;
#else
        union nodeptr p = (union nodeptr){ .ptr = n->parent };
        if (c == RBTREE_RED) {
                p.num |= 0x1U;
        } else {
                p.num &= ~(0x1U);
        }
        n->parent = p.ptr;
#endif
}

__const static enum rbtree_colour rbt_get_colour(struct rbtree_node *n)
{
        if (!n) {
                return (RBTREE_BLACK);
        }
#ifndef NDEBUG
        return (n->colour);
#else
        union nodeptr p = (union nodeptr){ .ptr = n->parent };
        enum rbtree_colour c = p.num & 0x1U;
        return (c);
#endif
}

static void rbt_set_parent(struct rbtree_node *node, struct rbtree_node *parent)
{
        kassert(node);

#ifndef NDEBUG
        node->parent = parent;
#else
        union nodeptr p = (union nodeptr){ .ptr = node->parent };
        p.num &= 0x1U;
        p.num |= (uintptr_t)parent;
        node->parent = p.ptr;
#endif
}

__const static struct rbtree_node *rbt_get_parent(struct rbtree_node *node)
{
        kassert(node);

#ifndef NDEBUG
        return (node->parent);
#else
        union nodeptr p = (union nodeptr){ .ptr = node->parent };
        p.num &= ~(0x1U);
        kassert(properly_aligned(p.ptr));
        return (p.ptr);
#endif
}

__const static struct rbtree_node *rbt_get_grandparent(struct rbtree_node *node)
{
        kassert(node);

        struct rbtree_node *parent = rbt_get_parent(node);
        kassert(parent);

        struct rbtree_node *grandparent = rbt_get_parent(parent);
        kassert(grandparent);

        return (grandparent);
}

__const static struct rbtree_node *rbt_get_sibling(struct rbtree_node *node)
{
        kassert(node);

        struct rbtree_node *parent = rbt_get_parent(node);
        kassert(parent);

        if (parent->left == node) {
                return (parent->right);
        } else {
                return (parent->left);
        }
}

static struct rbtree_node *rbt_get_uncle(struct rbtree_node *node)
{
        kassert(node);

        struct rbtree_node *parent = rbt_get_parent(node);
        kassert(parent);
        kassert(rbt_get_parent(parent));

        return rbt_get_sibling(parent);
}

static void rbt_replace_subtree(struct rbtree *rbt, struct rbtree_node *replacee,
                                struct rbtree_node *replacement)
{
        struct rbtree_node *replacee_parent = rbt_get_parent(replacee);
        if (replacee_parent) {
                /* Fix parent's reference. */
                if (replacee_parent->left == replacee) {
                        replacee_parent->left = replacement;
                } else {
                        replacee_parent->right = replacement;
                }
        } else {
                /* Replacement is a new root. */
                rbt->root = replacement;
        }
        if (replacement) {
                rbt_set_parent(replacement, replacee_parent);
        }
}

static void rbt_swap_nodes(struct rbtree *rbt, struct rbtree_node *x, struct rbtree_node *y)
{
#define TRY_SET_PARENT(NODE, PARENT)                      \
        do {                                              \
                if ((NODE) != NULL) {                     \
                        rbt_set_parent((NODE), (PARENT)); \
                }                                         \
        } while (0)

        kassert(rbt);
        kassert(x);
        kassert(y);

        /* If Xp == Y, swap them
         * So only Yp == X is possible.
         */
        if (rbt_get_parent(x) == y) {
                /* X node is always a parent of the node Y. */
                struct rbtree_node *tmp = y;
                y = x;
                x = tmp;
        }

        struct rbtree_node y_copy = *y;

        TRY_SET_PARENT(x->left, y);
        TRY_SET_PARENT(x->right, y);

        struct rbtree_node *Xp = rbt_get_parent(x);
        if (Xp) {
                if (Xp->left == x) {
                        Xp->left = y;
                } else {
                        Xp->right = y;
                }
                rbt_set_parent(y, Xp);
        } else {
                /* X was a root.
                 * Make Y a root.
                 */
                kassert(rbt->root == x);
                rbt_set_parent(y, NULL);
                rbt->root = y;
        }

        /* Also, handle special case when Y is a child of X. */
        y->left = x->left != y ? x->left : x;
        y->right = x->right != y ? x->right : x;
        rbt_set_colour(y, rbt_get_colour(x));

        TRY_SET_PARENT(y_copy.left, x);
        TRY_SET_PARENT(y_copy.right, x);

        struct rbtree_node *Yp = rbt_get_parent(&y_copy);
        if (Yp) {
                /* If Y was the child of X, then Y was already appropriately altered. */
                if (Yp != x) {
                        if (Yp->left == y) {
                                Yp->left = x;
                        } else {
                                Yp->right = x;
                        }
                        rbt_set_parent(x, rbt_get_parent(&y_copy));
                } else {
                        rbt_set_parent(x, y);
                }
        } else {
                /* Y was a root.
                 * Make X a root. */
                kassert(rbt->root == y);
                rbt_set_parent(x, NULL);
                rbt->root = x;
        }

        x->left = y_copy.left;
        x->right = y_copy.right;
        rbt_set_colour(x, rbt_get_colour(&y_copy));

#undef TRY_SET_PARENT

        /* Assertions. */
        if (x->left) {
                kassert(rbt_get_parent(x->left) == x);
        }
        if (x->right) {
                kassert(rbt_get_parent(x->right) == x);
        }
        if (rbt_get_parent(x)) {
                kassert(rbt_get_parent(x)->left == x || rbt_get_parent(x)->right == x);
        }

        if (y->left) {
                kassert(rbt_get_parent(y->left) == y);
        }
        if (y->right) {
                kassert(rbt_get_parent(y->right) == y);
        }
        if (rbt_get_parent(y)) {
                kassert(rbt_get_parent(y)->left == y || rbt_get_parent(y)->right == y);
        }
}

void rbtree_init_tree(struct rbtree *rbt)
{
        kassert(rbt);
        rbt->root = NULL;
}

void rbtree_init_node(struct rbtree_node *node)
{
        kassert(node);
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        rbt_set_colour(node, RBTREE_RED);
}

static void rbt_rotate_left(struct rbtree *rbt, struct rbtree_node *old_root)
{
        kassert(old_root);

        struct rbtree_node *new_root = old_root->right;
        rbt_replace_subtree(rbt, old_root, new_root);

        old_root->right = new_root->left;
        if (old_root->right) {
                rbt_set_parent(old_root->right, old_root);
        }
        new_root->left = old_root;
        rbt_set_parent(old_root, new_root);
}

static void rbt_rotate_right(struct rbtree *rbt, struct rbtree_node *old_root)
{
        kassert(old_root);

        struct rbtree_node *new_root = old_root->left;
        rbt_replace_subtree(rbt, old_root, new_root);

        old_root->left = new_root->right;
        if (old_root->left) {
                rbt_set_parent(old_root->left, old_root);
        }
        new_root->right = old_root;
        rbt_set_parent(old_root, new_root);
}

static void rbt_insert_fix(struct rbtree *rbt, struct rbtree_node *new)
{
        kassert(new);

        struct rbtree_node *node = new;
        while (true) {
                struct rbtree_node *parent = rbt_get_parent(node);
                if (!parent) {
                        /* New node is a new root. */
                        rbt_set_colour(node, RBTREE_BLACK);
                        break;
                }

                if (rbt_get_colour(parent) == RBTREE_BLACK) {
                        /* BLACK + RED => Tree is valid. */
                        break;
                }

                /* Parent and Uncle are red. */
                struct rbtree_node *uncle = rbt_get_uncle(node);
                if (rbt_get_colour(uncle) == RBTREE_RED) {
                        struct rbtree_node *grandparent = rbt_get_grandparent(node);
                        /* Grandparent can't be NULL because node's parent is red. */
                        kassert(grandparent);

                        rbt_set_colour(parent, RBTREE_BLACK);
                        rbt_set_colour(uncle, RBTREE_BLACK);
                        rbt_set_colour(grandparent, RBTREE_RED);

                        /* Current node should not violate any properties now.
                         * But grandparent may. */
                        node = grandparent;
                        continue;
                }

                /* Parent is red but an Uncle is black. Also G->P->N relation forms a triangle. */
                struct rbtree_node *grandparent = rbt_get_grandparent(node);
                kassert(grandparent);
                if ((node == parent->right) && (parent == grandparent->left)) {
                        rbt_rotate_left(rbt, parent);
                        node = node->left;
                } else if ((node == parent->left) && (parent == grandparent->right)) {
                        rbt_rotate_right(rbt, parent);
                        node = node->right;
                }
                /* Update after potential rotations. */
                parent = rbt_get_parent(node);
                grandparent = rbt_get_grandparent(node);
                kassert(parent);
                kassert(grandparent);

                /* Parent is red but an Uncle is black. But G->P->N relation forms a line. */
                if ((node == parent->left) && (parent == grandparent->left)) {
                        rbt_rotate_right(rbt, grandparent);
                } else if ((node == parent->right) && (parent == grandparent->right)) {
                        rbt_rotate_left(rbt, grandparent);
                } else {
                        /* How did we get here? */
                        kassert(false);
                }
                /* Grandparent becomes the parent's child. Strange times. */
                rbt_set_colour(parent, RBTREE_BLACK);
                rbt_set_colour(grandparent, RBTREE_RED);

                break;
        }
}

void rbtree_insert(struct rbtree *rbt, struct rbtree_node *new, rbtree_cmp_fn cmpf)
{
        kassert(rbt);
        kassert(new);

        rbtree_init_node(new);

        struct rbtree_node *parent = NULL;
        struct rbtree_node *cursor = rbt->root;
        /* It will hold a result of the last comparison, which will be used later. */
        int cmp_result = 0;
        while (cursor != NULL) {
                parent = cursor;
                cmp_result = cmpf(cursor->data, new->data);
                if (cmp_result >= 0) {
                        cursor = cursor->left;
                } else {
                        cursor = cursor->right;
                }
        }
        rbt_set_parent(new, parent);

        if (parent) {
                if (cmp_result >= 0) {
                        parent->left = new;
                } else {
                        parent->right = new;
                }
        } else {
                /* Tree is empty. */
                kassert(!rbt->root);
                rbt->root = new;
        }

        rbt_insert_fix(rbt, new);
}

static struct rbtree_node *rbt_find_successor(struct rbtree_node *subtree)
{
        struct rbtree_node *n = subtree->left;
        while (n->right) {
                n = n->right;
        }
        return (n);
}

static void rbt_delete_fix(struct rbtree *rbt, struct rbtree_node *node)
{
        while (true) {
                struct rbtree_node *parent = rbt_get_parent(node);
                if (!parent) {
                        /* We have removed black node from every path; no properties were violated. */
                        break;
                }

                struct rbtree_node *sibling = rbt_get_sibling(node);
                if (rbt_get_colour(sibling) == RBTREE_RED) {
                        kassert(rbt_get_colour(parent) == RBTREE_BLACK);

                        /* Convert to a one of the cases with a black sibling. */
                        if (parent->right == sibling) {
                                rbt_rotate_left(rbt, parent);
                        } else {
                                rbt_rotate_right(rbt, parent);
                        }
                        rbt_set_colour(sibling, RBTREE_BLACK);
                        rbt_set_colour(parent, RBTREE_RED);

                        /* Update after rotation. */
                        parent = rbt_get_parent(node);
                        sibling = rbt_get_sibling(node);
                }

                bool siblings_children_black = rbt_get_colour(sibling->left) == RBTREE_BLACK &&
                                               rbt_get_colour(sibling->right) == RBTREE_BLACK;
                if (rbt_get_colour(sibling) == RBTREE_BLACK && siblings_children_black) {
                        if (rbt_get_colour(parent) == RBTREE_BLACK) {
                                rbt_set_colour(sibling, RBTREE_RED);
                                node = rbt_get_parent(node);
                                continue;
                        } else {
                                rbt_set_colour(sibling, RBTREE_RED);
                                rbt_set_colour(parent, RBTREE_BLACK);
                                break;
                        }
                }

                if (rbt_get_colour(sibling) == RBTREE_BLACK) {
                        /* Sibling or node? */
                        if (parent->right == node &&
                            rbt_get_colour(sibling->left) == RBTREE_BLACK &&
                            rbt_get_colour(sibling->right) == RBTREE_RED) {
                                rbt_set_colour(sibling, RBTREE_RED);
                                rbt_set_colour(sibling->right, RBTREE_BLACK);
                                rbt_rotate_left(rbt, sibling);
                        } else if (parent->left == node &&
                                   rbt_get_colour(sibling->left) == RBTREE_RED &&
                                   rbt_get_colour(sibling->right) == RBTREE_BLACK) {
                                rbt_set_colour(sibling, RBTREE_RED);
                                rbt_set_colour(sibling->left, RBTREE_BLACK);
                                rbt_rotate_right(rbt, sibling);
                        }

                        /* Update after rotate. */
                        sibling = rbt_get_sibling(node);
                        parent = rbt_get_parent(node);
                }

                kassert(rbt_get_colour(sibling) == RBTREE_BLACK);
                kassert((rbt_get_colour(sibling->left) == RBTREE_RED && node == parent->right) ||
                        (rbt_get_colour(sibling->right) == RBTREE_RED && node == parent->left));

                rbt_set_colour(sibling, rbt_get_colour(parent));
                rbt_set_colour(parent, RBTREE_BLACK);
                if (parent->right == node) {
                        kassert(rbt_get_colour(sibling->left) == RBTREE_RED);
                        rbt_set_colour(sibling->left, RBTREE_BLACK);
                        rbt_rotate_right(rbt, parent);
                } else {
                        kassert(rbt_get_colour(sibling->right) == RBTREE_RED);
                        rbt_set_colour(sibling->right, RBTREE_BLACK);
                        rbt_rotate_left(rbt, parent);
                }

                break;
        }
}

void rbtree_delete(struct rbtree *rbt, struct rbtree_node *deletee)
{
        if (deletee->left != NULL && deletee->right != NULL) {
                struct rbtree_node *successor = rbt_find_successor(deletee);
                rbt_swap_nodes(rbt, deletee, successor);
        }

        kassert(deletee->left == NULL || deletee->right == NULL);

        struct rbtree_node *child = deletee->right ? deletee->right : deletee->left;
        /* When a child is black and a deletee is red, we don't need to do anything. */
        if (rbt_get_colour(deletee) == RBTREE_BLACK) {
                if (rbt_get_colour(child) == RBTREE_RED) {
                        rbt_set_colour(child, RBTREE_BLACK);
                } else {
                        rbt_delete_fix(rbt, deletee);
                }
        }
        rbt_replace_subtree(rbt, deletee, child);
}

struct rbtree_node *rbtree_search(struct rbtree *rbt, void *value, rbtree_cmp_fn cmpf)
{
        kassert(rbt);

        struct rbtree_node *node = rbt->root;
        while (node) {
                int c = cmpf(node->data, value);
                if (c == 0) {
                        break;
                } else if (c > 0) {
                        node = node->left;
                } else {
                        kassert(c < 0);
                        node = node->right;
                }
        }

        return (node);
}

struct rbtree_node *rbtree_search_min(struct rbtree *rbt, void *limit, rbtree_cmp_fn cmpf)
{
        kassert(rbt);
        kassert(limit);

        struct rbtree_node *cursor = rbt->root;

        while (cursor) {
                int cmp_result = cmpf(cursor->data, limit);

                while (cmp_result > 0 && cursor->left != NULL) {
                        cursor = cursor->left;
                        cmp_result = cmpf(cursor->data, limit);
                }

                while (cmp_result < 0 && cursor->right != NULL) {
                        /* We could overstep the limit, so try to return to the limit's boundaries. */
                        cursor = cursor->right;
                        cmp_result = cmpf(cursor->data, limit);
                }

                if (cmp_result == 0) {
                        break;
                }

                if (cmp_result < 0 && cursor->right == NULL) {
                        while (cmp_result < 0) {
                                cursor = rbt_get_parent(cursor);
                                cmp_result = cmpf(cursor->data, limit);
                        }
                        break;
                }

                if (cmp_result > 0 && cursor->left == NULL) {
                        break;
                }
        }

        return (cursor);
}

struct rbtree_node *rbtree_search_max(struct rbtree *rbt, void *limit, rbtree_cmp_fn cmpf)
{
        kassert(rbt);
        kassert(limit);

        struct rbtree_node *cursor = rbt->root;
        int cmp_result = 0;

        while (cursor != NULL) {
                cmp_result = cmpf(cursor->data, limit);

                /* Find rightmost node within the limit. */
                while (cmp_result < 0 && cursor->right != NULL) {
                        cursor = cursor->right;
                        cmp_result = cmpf(cursor->data, limit);
                }

                /* We could overstep the limit, so try to return to the limit's boundaries. */
                while (cmp_result > 0 && cursor->left != NULL) {
                        cursor = cursor->left;
                        cmp_result = cmpf(cursor->data, limit);
                }

                /* Found exact match. Just return it. */
                if (cmp_result == 0) {
                        break;
                }

                /* We want to step back to the parent if we've overstepped the limit
                 * and there is no left subtree. */
                if (cmp_result > 0 && cursor->left == NULL) {
                        while (cmp_result > 0 && rbt_get_parent(cursor) != NULL) {
                                cursor = rbt_get_parent(cursor);
                                cmp_result = cmpf(cursor->data, limit);
                        }
                        break;
                }

                /* It seems that we've found the closest possible match. */
                if (cmp_result < 0 && cursor->right == NULL) {
                        break;
                }
        }

        if (cmp_result > 0) {
                return (NULL);
        }
        return (cursor);
}

void rbtree_iter_range(struct rbtree *rbt, void *value_from, void *value_to, rbtree_cmp_fn cmpf,
                       bool (*fn)(void *elem, void *data), void *data)
{
        kassert(rbt);
        kassert(cmpf(value_from, value_to) < 0);

        struct rbtree_node *cursor = rbtree_search_min(rbt, value_from, cmpf);

        /* TODO: Rewrite. It's working fine but it's ugly.
         * A hacky way to start from the current node and not to iterate over
         * the left subtree which is smaller than the lower boundary.
         */
        struct rbtree_node *prev = cursor->left;
        while (cursor) {
                if (prev == cursor->left) {
                        int c = cmpf(cursor->data, value_to);
                        if (c > 0) {
                                return;
                        }

                        if (fn(cursor->data, data) == false) {
                                return;
                        }

                        prev = cursor;
                        if (cursor->right) {
                                cursor = cursor->right;
                        } else {
                                cursor = rbt_get_parent(cursor);
                        }
                } else if (prev == cursor->right) {
                        prev = cursor;
                        cursor = rbt_get_parent(cursor);
                } else {
                        if (cursor->left) {
                                prev = cursor;
                                cursor = cursor->left;
                        } else {
                                /* There is no left child. Pretend that we've already iterated over it. */
                                prev = cursor->left;
                        }
                }
        }
}
