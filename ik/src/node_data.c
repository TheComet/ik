#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node_data.h"
#include "ik/effector.h"
#include "ik/constraint.h"
#include "ik/pole.h"
#include <assert.h>
#include <string.h>

static void
destruct_node_data(struct ik_node_data_t* node_data)
{
    IK_XDECREF(node_data->effector);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_data_create(struct ik_node_data_t** node_data, const void* user_data)
{
    ikret_t status;

    assert(node_data);
    *node_data = MALLOC(sizeof **node_data);
    if (*node_data == NULL)
    {
        ik_log_fatal("Failed to allocate node_data: Ran out of memory");
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, malloc_failed);
    }

    memset(*node_data, 0, sizeof **node_data);

    /* node_data is a refcounted type so it can be safely casted to
     * ik_refcount_t. This initializes the refcount */
    if ((status = ik_refcount_create(&(*node_data)->refcount,
            (ik_destruct_func)destruct_node_data, 1)) != IK_OK)
        IK_FAIL(status, construct_failed);

    (*node_data)->user_data = user_data;
    (*node_data)->rotation_weight = 0.0;
    (*node_data)->dist_to_parent = 0.0;
    (*node_data)->mass = 0.0;
    ik_quat_set_identity((*node_data)->transform.t.rotation.f);
    ik_vec3_set_zero((*node_data)->transform.t.position.f);

    return IK_OK;

    construct_failed : FREE(*node_data);
    malloc_failed    : return status;
}

/* ------------------------------------------------------------------------- */
void
ik_node_data_destroy(struct ik_node_data_t* node_data)
{
    IK_DECREF(node_data);
}

/* ------------------------------------------------------------------------- */
void
ik_node_data_ref_members(struct ik_node_data_t* node_data)
{
    IK_XINCREF(node_data->effector);
    IK_XINCREF(node_data->constraint);
    IK_XINCREF(node_data->pole);
}
