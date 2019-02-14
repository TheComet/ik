#ifndef IK_REFCOUNTED_H
#define IK_REFCOUNTED_H

#include "ik/config.h"

C_BEGIN

#define IK_REFCOUNTED(Type)                             \
        void (*destroy)(Type* self);                    \
        struct ik_refcount_t* refcount;

#define IK_INCREF(o)                                    \
        o->refcount->refs++;

#define IK_DECREF(o) do {                               \
        assert(o->refcount->refs > 0);                  \
        if (--(o->refcount->refs))                      \
            o->destroy(o);                              \
    } while(0)

#define IK_XDECREF(o) do {                              \
        if (o)                                          \
            IK_DECREF(o);                               \
    } while(0)

typedef void (*ik_destroy_func)(void*);

struct ik_refcount_t
{
    uint32_t refs;
};

struct ik_refcounted_t
{
    ik_destroy_func destroy;
    struct ik_refcount_t* refcount;
};

IK_PRIVATE_API ikret_t
ik_refcounted_create(struct ik_refcounted_t* refcounted,
                     ik_destroy_func destroy);

IK_PRIVATE_API void
ik_refcounted_destroy(struct ik_refcounted_t* refcounted);

C_END

#endif /* IK_REFCOUNTED_H */
