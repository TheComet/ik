#include "ik/refcount.h"
#include "ik/memory.h"
#include "ik/log.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_refcount_create(struct ik_refcount_t** refcount,
                   ik_destruct_func destruct,
                   uint32_t array_length)
{
    *refcount = MALLOC(sizeof **refcount);
    if (*refcount == NULL)
    {
        ik_log_fatal("Failed to allocate refcounted: Ran out of memory");
        return IK_ERR_OUT_OF_MEMORY;
    }

    (*refcount)->destruct = destruct;
    (*refcount)->refs = 1;
    (*refcount)->array_length = array_length;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_refcount_destroy(struct ik_refcount_t* refcount)
{
    assert(refcount->refs == 0);
    FREE(refcount);
}
