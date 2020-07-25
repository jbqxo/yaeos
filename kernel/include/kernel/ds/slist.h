#ifndef _KERNEL_DS_SLIST_H
#define _KERNEL_DS_SLIST_H

#define SLIST_FIELD(type)                                                                          \
	struct {                                                                                   \
		struct type *next;                                                                 \
	} __slist_fieldv

#define SLIST_NODE_INIT(head)                                                                      \
	do {                                                                                       \
		SLIST_NEXT(head) = NULL;                                                           \
	} while (0)

#define SLIST_EMPTY(head) (SLIST_NEXT(head) == NULL)
#define SLIST_NEXT(var) ((var)->__slist_fieldv.next)

#define SLIST_FOREACH(head, iterv) for ((iterv) = (head); (iterv); (iterv) = SLIST_NEXT(iterv))

#define SLIST_ADD_AFTER(elem, newelem)                                                             \
	do {                                                                                       \
		SLIST_NEXT(newelem) = SLIST_NEXT(elem);                                            \
		SLIST_NEXT(elem) = (newelem);                                                      \
	} while (0)

#define SLIST_DEL_AFTER(elem)                                                                      \
	do {                                                                                       \
		SLIST_NEXT(elem) = SLIST_NEXT(SLIST_NEXT(elem));                                   \
	} while (0)

#endif // _KERNEL_DS_SLIST_H
