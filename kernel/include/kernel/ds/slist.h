#ifndef _KERNEL_DS_SLIST_H
#define _KERNEL_DS_SLIST_H

#define SLIST_HEAD(name, type)                                                                     \
	struct name {                                                                              \
		type *first;                                                                       \
	}

#define SLIST_FIELD(type)                                                                          \
	struct {                                                                                   \
		type *next;                                                                        \
	}

#define SLIST_NEXT(slist, field) ((slist)->field.next)
#define SLIST_FIRST(head) ((head)->first)
#define SLIST_EMPTY(head) (SLIST_FIRST(head) == NULL)

#define SLIST_INIT(head)                                                                           \
	do {                                                                                       \
		SLIST_FIRST(head) = NULL;                                                          \
	} while (0)

#define SLIST_FOREACH(iterv, head, field)                                                          \
	for ((iterv) = SLIST_FIRST(head); (iterv); (iterv) = SLIST_NEXT(iterv, field))

#define SLIST_INSERT_AFTER(elem, newelem, field)                                                   \
	do {                                                                                       \
		SLIST_NEXT(newelem, field) = SLIST_NEXT(elem, field);                              \
		SLIST_NEXT(elem, field) = (newelem);                                               \
	} while (0)

#define SLIST_INSERT_HEAD(head, newelem, field)                                                    \
	do {                                                                                       \
		SLIST_NEXT(newelem, field) = SLIST_FIRST(head);                                    \
		SLIST_FIRST(head) = newelem;                                                       \
	} while (0)

#define SLIST_REMOVE_AFTER(elem, field)                                                            \
	do {                                                                                       \
		SLIST_NEXT(elem, field) = SLIST_NEXT(SLIST_NEXT(elem, field), field);              \
	} while (0)

#define SLIST_REMOVE_HEAD(head, field)                                                             \
	do {                                                                                       \
		SLIST_FIRST(head) = SLIST_NEXT(SLIST_FIRST(head), field);                          \
	} while (0)

#define SLIST_FIELD_INIT(elem, field)                                                          \
	do {                                                                                       \
		SLIST_NEXT(elem, field) = NULL;                                                    \
	} while (0)

#define SLIST_REMOVE(head, elem, field)                                                            \
	do {                                                                                       \
		if (SLIST_FIRST(head) == (elem)) {                                                 \
			SLIST_REMOVE_HEAD(head, field);                                            \
			/* return; */                                                              \
			break;                                                                     \
		}                                                                                  \
		typeof(SLIST_FIRST(head)) current = SLIST_FIRST(head);                             \
		while (SLIST_NEXT(current, field) != (elem)) {                                     \
			current = SLIST_NEXT(current, field);                                      \
		}                                                                                  \
		SLIST_REMOVE_AFTER(current, field);                                                \
	} while (0)

#endif // _KERNEL_DS_SLIST_H
