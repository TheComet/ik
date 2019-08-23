#ifndef IK_REFCOUNT_H
#define IK_REFCOUNT_H

#include "ik/config.h"

C_BEGIN

#define IK_REFCOUNT_HEAD                                                      \
        struct ik_refcount_t* refcount;

#define IK_INCREF(o) IK_INCREF_((struct ik_refcounted_t*)o)
#define IK_INCREF_(o)                                                         \
        (o)->refcount->refs++;

#define IK_DECREF(o) IK_DECREF_((struct ik_refcounted_t*)o)
#define IK_DECREF_(o) do {                                                    \
        assert((o)->refcount->refs > 0);                                      \
        if (--((o)->refcount->refs) == 0)                                     \
        {                                                                     \
            uint32_t decref_i;                                                \
            struct ik_refcount_t* refcount = (o)->refcount;                   \
            for (decref_i = 0; refcount->array_length--; decref_i++)          \
                refcount->deinit((o) + decref_i);                             \
            FREE(o); /* future: refcount->free(o); */                         \
            ik_refcount_free(refcount);                                       \
        }                                                                     \
    } while(0)

#define IK_XINCREF(o) do {                                                    \
        if (o)                                                                \
            IK_INCREF(o);                                                     \
    } while(0)

#define IK_XDECREF(o) do {                                                    \
        if (o)                                                                \
            IK_DECREF(o);                                                     \
    } while(0)


typedef void (*ik_deinit_func)(void*);

struct ik_refcounted_t
{
    IK_REFCOUNT_HEAD
};

struct ik_refcount_t
{
    /* Handler for freeing data managed by the refcounted object */
    ik_deinit_func deinit;
    /* Reference count */
    uint32_t         refs;
    /* Number of contiguous objects pointing to this refcount */
    uint32_t         array_length;
};

IK_PRIVATE_API IKRET
ik_refcount_create(struct ik_refcount_t** refcount,
                   ik_deinit_func deinit,
                   uint32_t array_length);

IK_PRIVATE_API void
ik_refcount_free(struct ik_refcount_t* refcount);

C_END

#endif /* IK_REFCOUNT_H */
