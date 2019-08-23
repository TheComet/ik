#include "cstructures/memory.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include <string.h>
#include <assert.h>

static void
deinit_effector(struct ik_effector_t* effector)
{
    /* No data is managed by effector */
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_effector_create(struct ik_effector_t** effector)
{
    ikret_t status;

    *effector = MALLOC(sizeof **effector);
    if (effector == NULL)
    {
        ik_log_fatal("Failed to allocate effector: Not enough memory");
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, alloc_effector_failed);
    }

    memset(*effector, 0, sizeof **effector);

    if ((status = ik_refcount_create(&(*effector)->refcount,
            (ik_deinit_func)deinit_effector, 1)) != IK_OK)
        IK_FAIL(status, init_refcount_failed);

    ik_vec3_set_zero((*effector)->target_position.f);
    ik_quat_set_identity((*effector)->target_rotation.f);
    (*effector)->weight = 1.0;
    (*effector)->rotation_weight = 1.0;
    (*effector)->rotation_decay = 0.25;

    return IK_OK;

    init_refcount_failed  : FREE(*effector);
    alloc_effector_failed : return status;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_free(struct ik_effector_t* effector)
{
    IK_DECREF(effector);
}
