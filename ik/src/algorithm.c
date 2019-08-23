#include "cstructures/memory.h"
#include "ik/algorithm.h"
#include "ik/log.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include <string.h>
#include <assert.h>

static void
deinit_algorithm(struct ik_algorithm_t* algorithm)
{
    /* No data is managed by algorithm */
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_algorithm_create(struct ik_algorithm_t** algorithm)
{
    ikret_t status;

    *algorithm = MALLOC(sizeof **algorithm);
    if (algorithm == NULL)
    {
        ik_log_fatal("Failed to allocate algorithm: Not enough memory");
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, alloc_algorithm_failed);
    }

    memset(*algorithm, 0, sizeof **algorithm);

    if ((status = ik_refcount_create(&(*algorithm)->refcount,
            (ik_deinit_func)deinit_algorithm, 1)) != IK_OK)
        IK_FAIL(status, init_refcount_failed);

    (*algorithm)->tolerance = 1e-2;
    (*algorithm)->max_iterations = 20;
    (*algorithm)->features = IK_SOLVER_JOINT_ROTATIONS;
    (*algorithm)->type = IK_SOLVER_ONE_BONE;

    return IK_OK;

    init_refcount_failed  : FREE(*algorithm);
    alloc_algorithm_failed : return status;
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_free(struct ik_algorithm_t* algorithm)
{
    IK_DECREF(algorithm);
}
