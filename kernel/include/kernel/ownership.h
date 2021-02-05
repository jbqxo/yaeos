#ifndef _KERNEL_OWNERSHIP_H
#define _KERNEL_OWNERSHIP_H

/* Certainly, there is ought to be a better name.
 * TODO: Rename "Ownership" to something better. */

/* Helps to track ownership of the object it's embedded to. */
struct ownership {
        void *owner;
};

void ownership_init(struct ownership *);

void ownership_add(struct ownership *, void *owner);

void *ownership_get(struct ownership *);

#endif /* _KERNEL_OWNERSHIP_H */
