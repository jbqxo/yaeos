#ifndef _KERNEL_DS_SLIST_H
#define _KERNEL_DS_SLIST_H

#define SLIST_HEAD(name, type) \
        struct name {          \
                type *first;   \
        }

#define SLIST_FIELD(type)   \
        struct {            \
                type *next; \
        }

#define SLIST_NEXT(node, fieldname) ((node)->fieldname.next)
#define SLIST_FIRST(head)           ((head)->first)
#define SLIST_EMPTY(head)           (SLIST_FIRST(head) == NULL)

#define SLIST_CLEAR(node) ((node)->next = NULL)
#define SLIST_INIT(head)                  \
        do {                              \
                SLIST_FIRST(head) = NULL; \
        } while (0)

// TODO: Allow to start from a specific node.
#define SLIST_FOREACH(iterv, head, fieldname) \
        for ((iterv) = SLIST_FIRST(head); (iterv); (iterv) = SLIST_NEXT(iterv, fieldname))

#define SLIST_INSERT_AFTER(elem, newelem, fieldname)                          \
        do {                                                                  \
                SLIST_NEXT(newelem, fieldname) = SLIST_NEXT(elem, fieldname); \
                SLIST_NEXT(elem, fieldname) = (newelem);                      \
        } while (0)

#define SLIST_INSERT_HEAD(head, newelem, fieldname)                 \
        do {                                                        \
                SLIST_NEXT(newelem, fieldname) = SLIST_FIRST(head); \
                SLIST_FIRST(head) = newelem;                        \
        } while (0)

#define SLIST_REMOVE_AFTER(elem, fieldname)                                                       \
        do {                                                                                      \
                SLIST_NEXT(elem, fieldname) = SLIST_NEXT(SLIST_NEXT(elem, fieldname), fieldname); \
        } while (0)

#define SLIST_REMOVE_HEAD(head, fieldname)                                    \
        do {                                                                  \
                SLIST_FIRST(head) = SLIST_NEXT(SLIST_FIRST(head), fieldname); \
        } while (0)

#define SLIST_FIELD_INIT(elem, fieldname)           \
        do {                                        \
                SLIST_NEXT(elem, fieldname) = NULL; \
        } while (0)

#define SLIST_REMOVE(head, elem, fieldname)                            \
        do {                                                           \
                if (SLIST_FIRST(head) == (elem)) {                     \
                        SLIST_REMOVE_HEAD(head, fieldname);            \
                        /* return; */                                  \
                        break;                                         \
                }                                                      \
                typeof(SLIST_FIRST(head)) current = SLIST_FIRST(head); \
                while (SLIST_NEXT(current, fieldname) != (elem)) {     \
                        current = SLIST_NEXT(current, fieldname);      \
                }                                                      \
                SLIST_REMOVE_AFTER(current, fieldname);                \
        } while (0)

#endif // _KERNEL_DS_SLIST_H
