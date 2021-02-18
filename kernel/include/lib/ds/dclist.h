#ifndef _KERNEL_DS_DCLIST_H
#define _KERNEL_DS_DCLIST_H

#define DCLIST_HEAD(name, type) \
        struct name {           \
                type *first;    \
        }

#define DCLIST_FIELD(type)  \
        struct {            \
                type *next; \
                type *prev; \
        }

#define DCLIST_INIT(head)                  \
        do {                               \
                DCLIST_FIRST(head) = NULL; \
        } while (0)

#define DCLIST_FIELD_INIT(elem, fieldname)           \
        do {                                         \
                DCLIST_NEXT(elem, fieldname) = NULL; \
                DCLIST_PREV(elem, fieldname) = NULL; \
        } while (0)

#define DCLIST_PREV(node, fieldname) ((node)->fieldname.prev)
#define DCLIST_NEXT(node, fieldname) ((node)->fieldname.next)
#define DCLIST_FIRST(head)           ((head)->first)
#define DCLIST_EMPTY(head)           (DCLIST_FIRST(head) == NULL)

#define DCLIST_FOREACH_FORWARD(iterv, from_node, fieldname) \
        for ((iterv) = (from_node); (iterv) != NULL; (iterv) = DCLIST_NEXT((iterv), (fieldname)))

#define DCLIST_FOREACH_BACKWARD(iterv, from_node, fieldname) \
        for ((iterv) = (from_node); (iterv) != NULL; (iterv) = DCLIST_PREV((iterv), (fieldname)))

#define DCLIST_INSERT_AFTER(elem, insertee, fieldname)                             \
        do {                                                                       \
                DCLIST_NEXT(insertee, fieldname) = DCLIST_NEXT(elem, fieldname);   \
                DCLIST_PREV(insertee, fieldname) = (elem);                         \
                DCLIST_PREV(DCLIST_NEXT(elem, fieldname), fieldname) = (insertee); \
                DCLIST_NEXT(elem, fieldname) = (insertee);                         \
        } while (0)

#define DCLIST_INSERT_BEFORE(elem, insertee, fieldname)                            \
        do {                                                                       \
                DCLIST_NEXT(insertee, fieldname) = (elem);                         \
                DCLIST_PREV(insertee, fieldname) = DCLIST_PREV(elem, fieldname);   \
                DCLIST_NEXT(DCLIST_PREV(elem, fieldname), fieldname) = (insertee); \
                DCLIST_PREV(elem, fieldname) = (insertee);                         \
        } while (0)

#endif // _KERNEL_DS_DCLIST_H
