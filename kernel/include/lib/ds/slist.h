#ifndef _LIB_DS_SLIST_H
#define _LIB_DS_SLIST_H

#include <stdbool.h>

struct slist_ref {
        struct slist_ref *next;
};

#define SLIST_REF_EMPTY ((struct slist_ref){ .next = NULL })

void slist_init(struct slist_ref *node);

struct slist_ref *slist_next(struct slist_ref const *node);

bool slist_is_empty(struct slist_ref const *node);

#define SLIST_FOREACH(iterv, first)                                      \
        for (struct slist_ref const *(iterv) = (first); (iterv) != NULL; \
             (iterv) = slist_next(iterv))

void slist_insert(struct slist_ref *prev_node, struct slist_ref *new_node);

void slist_remove_next(struct slist_ref *prev_node);

void slist_remove(struct slist_ref *list_head, struct slist_ref const *to_remove);

#endif /* _LIB_DS_SLIST_H */
