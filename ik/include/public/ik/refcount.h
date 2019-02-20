#ifndef IK_REFCOUNT_H
#define IK_REFCOUNT_H

#include "ik/config.h"

C_BEGIN

#define IK_REFCOUNTED(Type)                                 \
        struct ik_refcount_t* refcount;

#define IK_INCREF(o)                                        \
        (o)->refcount->refs++;

#define IK_DECREF(o) do {                                   \
        assert((o)->refcount->refs > 0);                    \
        if (--((o)->refcount->refs) == 0)                   \
        {                                                   \
            uint32_t i;                                     \
            struct ik_refcount_t* refcount = (o)->refcount; \
            for (i = 0; refcount->array_length--; i++)      \
                refcount->destruct((o) + i);                \
            FREE(o); /* future: refcount->destroy(o); */    \
            ik_refcount_destroy(refcount);                  \
        }                                                   \
    } while(0)

#define IK_XINCREF(o) do {                                  \
        if (o)                                              \
            IK_INCREF(o);                                   \
    } while(0)

#define IK_XDECREF(o) do {                                  \
        if (o)                                              \
            IK_DECREF(o);                                   \
    } while(0)

typedef void (*ik_destruct_func)(void*);

struct ik_refcount_t
{
    ik_destruct_func destruct;
    uint32_t refs;
    uint32_t array_length;
};

IK_PRIVATE_API ikret_t
ik_refcount_create(struct ik_refcount_t** refcount,
                   ik_destruct_func destruct,
                   uint32_t array_length);

IK_PRIVATE_API void
ik_refcount_destroy(struct ik_refcount_t* refcount);

C_END

#endif /* IK_REFCOUNT_H */
