#include "cstructures/memory.h"
#include "ik/refcount.h"
#include "ik/ik.h"
#include <stddef.h>
#include <assert.h>

/*!
 * Bytes to subtract from a refcount allocated memory block to get to the
 * refcount structure.
 */
#define IK_REFCOUNT_OFFSET \
        IK_ALIGN_TO_CPU_WORD_SIZE(sizeof(struct ik_refcount))

/* ------------------------------------------------------------------------- */
static void
dummy_deinit(void* o)
{
    (void)o;
}

/* ------------------------------------------------------------------------- */
struct ik_refcounted*
ik_refcounted_alloc_array(uintptr_t obj_size,
                          ik_deinit_func deinit,
                          uint32_t obj_count)
{
    uintptr_t i;
    const uintptr_t refcount_size = IK_REFCOUNT_OFFSET;
    struct ik_refcount* refcount = MALLOC(refcount_size + obj_size * obj_count);
    if (refcount == NULL)
    {
        ik_log_out_of_memory("ik_refcounted_alloc_array()");
        return NULL;
    }

    refcount->refs = 0;
    refcount->deinit = deinit ? deinit : dummy_deinit;
    refcount->obj_count = obj_count;

    for (i = 0; i != obj_count; ++i)
    {
        struct ik_refcounted* obj = (struct ik_refcounted*)
            ((uintptr_t)refcount + refcount_size + i * obj_size);
        obj->refcount = refcount;
    }

    return ik_refcount_to_first_obj_address(refcount);
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
ik_refcounted_obj_free(struct ik_refcounted* refcounted_obj)
{
    FREE(ik_refcounted_obj_base_address(refcounted_obj));
}

void
ik_refcount_free(struct ik_refcount* refcount)
{
    FREE(refcount);
}

/* ------------------------------------------------------------------------- */
struct ik_refcount*
ik_refcounted_obj_base_address(struct ik_refcounted* refcounted_obj)
{
    return (struct ik_refcount*)((uintptr_t)refcounted_obj - IK_REFCOUNT_OFFSET);
}

/* ------------------------------------------------------------------------- */
struct ik_refcounted*
ik_refcount_to_first_obj_address(struct ik_refcount* refcount)
{
    return (struct ik_refcounted*)((uintptr_t)refcount + IK_REFCOUNT_OFFSET);
}
