#include "ik/refcounted.h"
#include "ik/memory.h"
#include "ik/log.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_refcounted_create(struct ik_refcounted_t* refcounted,
                     ik_destroy_func destroy)
{
    refcounted->destroy = destroy;
    refcounted->refcount = MALLOC(sizeof *(refcounted->refcount));
    if (refcounted->refcount == NULL)
    {
        ik_log_fatal("Failed to allocate refcounted: Ran out of memory");
        return IK_ERR_OUT_OF_MEMORY;
    }

    refcounted->refcount->refs = 1;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_refcounted_destroy(struct ik_refcounted_t* refcounted)
{
    assert(refcounted->refcount->refs == 0);
    FREE(refcounted);
}
