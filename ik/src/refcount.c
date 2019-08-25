#include "cstructures/memory.h"
#include "ik/refcount.h"
#include "ik/log.h"
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
        IK_ALIGN_TO_CPU_WORD_SIZE(sizeof(struct ik_refcount_t))

/* ------------------------------------------------------------------------- */
ikret_t
ik_refcount_malloc_array(struct ik_refcounted_t** refcounted_obj,
                         uintptr_t bytes,
                         ik_deinit_func deinit,
                         uint32_t array_length)
{
    uintptr_t refcount_size = IK_REFCOUNT_OFFSET;
    struct ik_refcount_t* head = MALLOC(refcount_size + bytes * array_length);
    if (*refcounted_obj == NULL)
    {
        ik_log_fatal("Failed to allocate refcounted memory: Ran out of memory");
        return IK_ERR_OUT_OF_MEMORY;
    }

    head->refs = 1;
    head->deinit = deinit;
    head->array_length = array_length;

    *refcounted_obj = (struct ik_refcounted_t*)((uintptr_t)head + refcount_size);
    (*refcounted_obj)->refcount = head;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_refcount_malloc(struct ik_refcounted_t** refcounted_obj,
                   uintptr_t bytes,
                   ik_deinit_func deinit)
{
    return ik_refcount_malloc_array(refcounted_obj, bytes, deinit, 1);
}
