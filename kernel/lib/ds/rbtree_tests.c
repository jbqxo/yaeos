// UNITY_TEST DEPENDS ON: kernel/test_fakes/panic.c

// TODO: Is it good idea for unit testing?
#include "rbtree.c"

#include "lib/cppdefs.h"
#include "lib/ds/rbtree.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unity.h>

// As we are using random numbers in a tree deletion, we run tests multiple times.
static const unsigned RANDOM_ITERS = 50;

void setUp(void)
{
        intmax_t used_seed = time(NULL);
        TEST_PRINTF("Used seed: %d", used_seed);
        srand(used_seed);
}

void tearDown(void)
{}

#define GET_DATA(NODE) (*(int *)(NODE)->data)
static int intcmp(void *_x, void *_y)
{
        const int x = *(const int *)_x;
        const int y = *(const int *)_y;
        return (x - y);
}

static int testset[] = {
        87, 31, 57, 97, 56, 80, 27, 97, 80, 34, 74, 3,  60, 79, 64, 75, 27, 26,  1,  82, 81, 75,
        49, 10, 85, 46, 11, 95, 63, 19, 28, 68, 76, 51, 74, 83, 28, 56, 12, 67,  86, 31, 26, 100,
        41, 80, 55, 89, 54, 4,  46, 6,  63, 96, 71, 81, 68, 94, 73, 86, 18, 50,  58, 91, 27, 93,
        38, 86, 92, 44, 18, 43, 46, 19, 29, 3,  92, 44, 38, 57, 80, 81, 91, 31,  39, 17, 82, 86,
        9,  9,  70, 38, 74, 22, 3,  76, 37, 13, 37, 96, 1,  26, 90, 6,  51, 90,  72, 90, 24, 55,
        93, 65, 5,  80, 4,  77, 11, 31, 50, 20, 29, 84, 5,  39, 98, 6,  66, 85,  41, 96, 98, 50,
        84, 4,  66, 34, 90, 2,  21, 70, 97, 86, 26, 89, 79, 13, 21, 93, 92, 19,  93, 81, 63, 24,
        60, 99, 16, 63, 58, 80, 40, 91, 90, 80, 98, 28, 68, 11, 80, 29, 59, 2,   69, 82, 16, 67,
        59, 12, 27, 55, 96, 37, 6,  81, 31, 7,  79, 43, 30, 42, 7,  91, 76, 49,  35, 78, 76, 76,
        24, 37, 78, 9,  13, 63, 24, 96, 11, 3,  66, 85, 96, 96, 96, 47, 86, 29,  40, 75, 42, 46,
        17, 39, 95, 73, 72, 15, 54, 55, 5,  32, 92, 93, 28, 81, 41, 77, 17, 20,  32, 15, 96, 31,
        12, 54, 80, 94, 63, 57, 22, 34, 99, 20, 41, 21, 16, 87, 67, 4,  43, 93,  19, 8,  17, 18,
        21, 70, 14, 33, 2,  20, 60, 40, 35, 39, 38, 74, 61, 15, 61, 64, 85, 73,  66, 93, 14, 24,
        62, 31, 86, 84, 44, 5,  55, 9,  42, 51, 88, 98, 21, 42, 86, 31, 2,  76,  39, 88, 18, 47,
        8,  36, 94, 70, 2,  42, 57, 14, 45, 20, 37, 71, 22, 36, 17, 34, 18, 72,  74, 63, 36, 73,
        74, 18, 71, 74, 56, 72, 4,  70, 38, 80, 36, 16, 9,  39, 90, 43, 92, 36,  84, 41, 35, 55,
        15, 47, 10, 96, 46, 87, 29, 35, 95, 63, 84, 16, 74, 70, 20, 22, 98, 59,  92, 72, 41, 72,
        56, 43, 61, 64, 37, 37, 16, 44, 39, 17, 80, 16, 21, 58, 99, 35, 93, 100, 23, 29, 44, 98,
        60, 61, 70, 62, 77, 45, 9,  50, 20, 97, 88, 73, 30, 53, 78, 8,  69, 41,  58, 79, 64, 71,
        26, 83, 52, 97, 40, 52, 21, 56, 24, 82, 34, 77, 81, 28, 69, 86, 58, 59,  71, 2,  14, 74,
        41, 9,  98, 52, 63, 66, 18, 23, 17, 77, 52, 72, 83, 95, 11, 97, 72, 14,  41, 92, 90, 60,
        53, 49, 76, 3,  14, 28, 33, 71, 18, 62, 75, 40, 59, 74, 64, 21, 43, 51,  49, 49, 94, 33,
        62, 67, 61, 71, 16, 49, 6,  15, 4,  45, 67, 71, 9,  81, 86, 24, 86, 64,  12, 49, 7,  23,
        90, 62
};
static const size_t testset_len = ARRAY_SIZE(testset);

static struct rbtree *create_tree_calling_back(void (*on_node_creation)(struct rbtree_node *root))
{
        struct rbtree *rbt = malloc(sizeof(*rbt));
        rbtree_init_tree(rbt, intcmp);
        for (int i = 0; i < testset_len; i++) {
                struct rbtree_node *new_node = malloc(sizeof(*new_node));
                new_node->data = &testset[i];

                rbtree_insert(rbt, new_node);
                if (on_node_creation) {
                        on_node_creation(rbt->root);
                }
        }

        TEST_ASSERT_MESSAGE(rbt->root, "Created tree is empty");
        return (rbt);
}

static void delete_random_elem(struct rbtree *rbt, struct rbtree_node *subroot)
{
        // To use random isn't very good idea.
        // TODO: Make proper test cases for RBT deletion.

        while (true) {
                // 0 - step left
                // 1 - delete
                // 2 - step right
                unsigned action = rand() % 3;
                switch (action) {
                case 0: {
                        if (subroot->left) {
                                delete_random_elem(rbt, subroot->left);
                                return;
                        }
                } break;
                case 1: {
                        rbtree_delete(rbt, subroot);
                        free(subroot);
                        return;
                } break;
                case 2: {
                        if (subroot->right) {
                                delete_random_elem(rbt, subroot->right);
                                return;
                        }
                } break;
                default: {
                        TEST_ASSERT_MESSAGE(action > 2, "Unknown action");
                } break;
                }
        }
}

static void randomly_delete_tree_withcb(struct rbtree *rbt,
                                        void (*on_node_deletion)(struct rbtree_node *root))
{
        while (rbt->root) {
                delete_random_elem(rbt, rbt->root);
                if (on_node_deletion) {
                        on_node_deletion(rbt->root);
                }
        }
        free(rbt);
}

static void delete_tree_withcb(struct rbtree *rbt,
                               void (*on_node_deletion)(struct rbtree_node *root))
{
        for (int i = 0; i < testset_len; i++) {
                struct rbtree_node *n = rbtree_search(rbt, &testset[i]);
                TEST_ASSERT_NOT_NULL(n);
                rbtree_delete(rbt, n);
                free(n);
                on_node_deletion(rbt->root);
        }
        free(rbt);
}

static void correct_bst_cb(struct rbtree_node *node)
{
        if (!node) {
                return;
        }

        if (node->left) {
                TEST_ASSERT_MESSAGE(GET_DATA(node) >= GET_DATA(node->left), "BST is invalid");
                correct_bst_cb(node->left);
        }
        if (node->right) {
                TEST_ASSERT_MESSAGE(GET_DATA(node) <= GET_DATA(node->right), "BST is invalid");
                correct_bst_cb(node->right);
        }
}

static void correct_bst(void)
{
        struct rbtree *rbt = create_tree_calling_back(correct_bst_cb);
        delete_tree_withcb(rbt, correct_bst_cb);
        for (int i = 0; i < RANDOM_ITERS; i++) {
                rbt = create_tree_calling_back(correct_bst_cb);
                randomly_delete_tree_withcb(rbt, correct_bst_cb);
        }
}

static void every_node_red_black_cb(struct rbtree_node *node)
{
        if (!node) {
                return;
        }
        TEST_ASSERT_MESSAGE(rbt_get_colour(node) == RBTREE_RED ||
                                    rbt_get_colour(node) == RBTREE_BLACK,
                            "Found unspecified colour in RBT");

        if (node->left) {
                every_node_red_black_cb(node->left);
        }
        if (node->right) {
                every_node_red_black_cb(node->right);
        }
}

static void every_node_red_black(void)
{
        struct rbtree *rbt = create_tree_calling_back(every_node_red_black_cb);
        delete_tree_withcb(rbt, every_node_red_black_cb);
        for (int i = 0; i < RANDOM_ITERS; i++) {
                rbt = create_tree_calling_back(every_node_red_black_cb);
                randomly_delete_tree_withcb(rbt, every_node_red_black_cb);
        }
}

static void root_black_cb(struct rbtree_node *root)
{
        TEST_ASSERT_MESSAGE(rbt_get_colour(root) == RBTREE_BLACK, "Root node isn't black");
}

static void root_black(void)
{
        struct rbtree *rbt = create_tree_calling_back(root_black_cb);
        delete_tree_withcb(rbt, root_black_cb);
        for (int i = 0; i < RANDOM_ITERS; i++) {
                rbt = create_tree_calling_back(root_black_cb);
                randomly_delete_tree_withcb(rbt, root_black_cb);
        }
}

static void red_node_has_black_children_cb(struct rbtree_node *node)
{
        if (!node) {
                return;
        }

        if (rbt_get_colour(node) == RBTREE_RED) {
                TEST_ASSERT_MESSAGE(rbt_get_colour(node->left) == RBTREE_BLACK &&
                                            rbt_get_colour(node->right) == RBTREE_BLACK,
                                    "Red node has red child");
        }
        red_node_has_black_children_cb(node->left);
        red_node_has_black_children_cb(node->right);
}

static void red_node_has_black_children(void)
{
        struct rbtree *rbt = create_tree_calling_back(red_node_has_black_children_cb);
        delete_tree_withcb(rbt, red_node_has_black_children_cb);
        for (int i = 0; i < RANDOM_ITERS; i++) {
                rbt = create_tree_calling_back(red_node_has_black_children_cb);
                randomly_delete_tree_withcb(rbt, red_node_has_black_children_cb);
        }
}

static int same_black_height_fn(struct rbtree_node *n)
{
        if (!n) {
                return (1);
        }

        int left_height = same_black_height_fn(n->left);
        int right_height = same_black_height_fn(n->right);
        TEST_ASSERT_MESSAGE(left_height == right_height, "Unequal black height");

        return (left_height + rbt_get_colour(n) == RBTREE_RED ? 0 : 1);
}

static void same_black_height_cb(struct rbtree_node *root)
{
        same_black_height_fn(root);
}

static void same_black_height(void)
{
        struct rbtree *rbt = create_tree_calling_back(same_black_height_cb);
        delete_tree_withcb(rbt, same_black_height_cb);
        for (int i = 0; i < RANDOM_ITERS; i++) {
                struct rbtree *rbt = create_tree_calling_back(same_black_height_cb);
                randomly_delete_tree_withcb(rbt, same_black_height_cb);
        }
}

static const int can_iterate_low = 11;
static const int can_iterate_high = 99;
static const int can_iterate_markval = -1;

static bool can_iterate_fn(void *elem, void *data)
{
        int e = *(int *)elem;
        TEST_ASSERT_GREATER_OR_EQUAL_INT(can_iterate_low, e);
        TEST_ASSERT_LESS_OR_EQUAL_INT(can_iterate_high, e);

        *(int *)elem = can_iterate_markval;

        return (true);
}

static void can_iterate(void)
{
        int testset[] = { 0, 10, 15, 16, 17, 18, 20, 21, 99, 101, 115 };
        const size_t testset_len = ARRAY_SIZE(testset);
        struct rbtree *rbt = malloc(sizeof(*rbt));
        rbtree_init_tree(rbt, intcmp);

        for (int i = 0; i < testset_len; i++) {
                struct rbtree_node *n = malloc(sizeof(*n));
                rbtree_init_node(n);
                n->data = (void *)&testset[i];

                rbtree_insert(rbt, n);
        }

        rbtree_iter_range(rbt, (void *)&can_iterate_low, (void *)&can_iterate_high, can_iterate_fn,
                          NULL);

        for (int i = 0; i < testset_len; i++) {
                if (testset[i] != -1) {
                        TEST_ASSERT(testset[i] < can_iterate_low || testset[i] > can_iterate_high);
                }
        }
}

static void search_min(void)
{
        int testset[] = { 0, 8, 10, 35, 40 };
        const size_t testset_len = ARRAY_SIZE(testset);
        struct rbtree *rbt = calloc(1, sizeof(*rbt));
        rbtree_init_tree(rbt, intcmp);

        for (int i = 0; i < testset_len; i++) {
                struct rbtree_node *n = malloc(sizeof(*n));
                rbtree_init_node(n);
                n->data = &testset[i];

                rbtree_insert(rbt, n);
        }

        int minlimit = 9;
        struct rbtree_node *result = rbtree_search_min(rbt, &minlimit);

        TEST_ASSERT_EQUAL_INT_MESSAGE(10, *(int *)result->data, "We've found a wrong node");
}

static void search_max(void)
{
        int testset[] = { 0, 8, 10, 35, 40 };
        const size_t testset_len = ARRAY_SIZE(testset);
        struct rbtree *rbt = calloc(1, sizeof(*rbt));
        rbtree_init_tree(rbt, intcmp);

        for (int i = 0; i < testset_len; i++) {
                struct rbtree_node *n = malloc(sizeof(*n));
                rbtree_init_node(n);
                n->data = &testset[i];

                rbtree_insert(rbt, n);
        }

        int maxlimit = 39;
        struct rbtree_node *result = rbtree_search_max(rbt, &maxlimit);

        TEST_ASSERT_EQUAL_INT_MESSAGE(35, *(int *)result->data, "We've found a wrong node");
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(correct_bst);
        RUN_TEST(every_node_red_black);
        RUN_TEST(root_black);
        RUN_TEST(red_node_has_black_children);
        RUN_TEST(same_black_height);
        RUN_TEST(can_iterate);
        RUN_TEST(search_min);
        RUN_TEST(search_max);
        UNITY_END();
        return (0);
}
