#include "lib/ds/slist.h"

#include "lib/cstd/assert.h"

#include <stdbool.h>
#include <stddef.h>

void slist_init(struct slist_ref *node)
{
        kassert(node != NULL);
        node->next = NULL;
}
struct slist_ref *slist_next(struct slist_ref const *node)
{
        kassert(node != NULL);
        return (node->next);
}

bool slist_is_empty(struct slist_ref const *node)
{
        kassert(node != NULL);
        return (node->next == NULL);
}

void slist_insert(struct slist_ref *prev_node, struct slist_ref *new_node)
{
        kassert(prev_node != NULL);
        kassert(new_node != NULL);

        new_node->next = prev_node->next;
        prev_node->next = new_node;
}

void slist_remove_next(struct slist_ref *prev_node)
{
        kassert(prev_node != NULL);
        kassert(prev_node->next != NULL);

        prev_node->next = prev_node->next->next;
}

void slist_remove(struct slist_ref *list_head, struct slist_ref const *to_remove)
{
        kassert(list_head != NULL);
        kassert(to_remove != NULL);

        struct slist_ref **cursor = &list_head->next;
        while (*cursor != to_remove) {
                cursor = &(*cursor)->next;
                kassert(*cursor != NULL);
        }

        *cursor = (*cursor)->next;
}
