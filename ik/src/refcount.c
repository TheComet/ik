#include "cstructures/memory.h"
#include "ik/refcount.h"
#include "ik/ik.h"
#include <stddef.h>
#include <assert.h>

/* TODO: Move this somewhere else and don't fix it to 64-bit */
#define IK_ALIGN_TO_CPU_WORD_SIZE(offset) \
        (((offset) & 0x7) == 0 ? (offset) : ((offset) & ~0x7) + 8)

/*!
 * Bytes to subtract from a refcount allocated memory block to get to the
 * refcount structure.
 */
#define IK_REFCOUNT_OFFSET \
        IK_ALIGN_TO_CPU_WORD_SIZE(sizeof(struct ik_refcount))

/* ------------------------------------------------------------------------- */
struct ik_refcounted*
ik_refcounted_alloc_array(uintptr_t obj_size,
                          ik_deinit_func deinit,
                          uint32_t obj_count)
{
    struct ik_refcounted* obj;
    uintptr_t refcount_size = IK_REFCOUNT_OFFSET;
    struct ik_refcount* refcount = MALLOC(refcount_size + obj_size * obj_count);
    if (refcount == NULL)
    {
        ik_log_out_of_memory("ik_refcounted_alloc_array()");
        return NULL;
    }

    refcount->refs = 1;
    refcount->deinit = deinit;
    refcount->obj_count = obj_count;

    obj = (struct ik_refcounted*)((uintptr_t)refcount + refcount_size);
    obj->refcount = refcount;

    return obj;
}

/* ------------------------------------------------------------------------- */
struct ik_refcounted*
ik_refcounted_alloc(uintptr_t bytes,
                    ik_deinit_func deinit)
{
    return ik_refcounted_alloc_array(bytes, deinit, 1);
}

/* ------------------------------------------------------------------------- */
void
ik_refcounted_free(struct ik_refcounted* refcounted_obj)
{
    FREE(refcounted_obj->refcount);
}
