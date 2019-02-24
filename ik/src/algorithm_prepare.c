#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/ntf.h"
#include "ik/node_data.h"
#include "ik/algorithm_prepare.h"
#include "ik/algorithm.h"
#include "ik/transform.h"
#include "ik/vector.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_algorithm_prepare_stack_buffer(struct ik_algorithm_t* algorithm)
{
    uint32_t max_children;

    /*
     * The algorithm needs a small stack to push/pop transformations as it
     * iterates the tree.
     * TODO: Add support for alloca(). If the stack is small enough and the
     * platform supports alloca(), leave this as NULL.
     */

    XFREE(algorithm->stack_buffer);
    algorithm->stack_buffer = NULL;

    /* Simple trees don't have more than 1 child */
    max_children = ik_ntf_find_highest_child_count(algorithm->ntf);
    if (max_children <= 1)
        return IK_OK;

    algorithm->stack_buffer = MALLOC(sizeof(union ik_transform_t) * max_children);
    if (algorithm->stack_buffer == NULL)
    {
        ik_log_fatal("Failed to allocate algorithm stack: Ran out of memory");
        return IK_ERR_OUT_OF_MEMORY;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_prepare_pole_targets(struct ik_algorithm_t* algorithm)
{
}
