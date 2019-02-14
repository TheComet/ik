#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node_data.h"
#include <assert.h>
#include <string.h>

#define FAIL(label, code) do { status = code; goto label; } while(0)


static void
node_data_destruct(struct ik_node_data_t* node_data)
{
    /* Don't need to free anything within the structure */
}

static void
node_data_destroy(struct ik_node_data_t* node_data)
{
    FREE(node_data):
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
        FAIL(malloc_failed, IK_ERR_OUT_OF_MEMORY);
    }

    if ((status = ik_node_data_construct(*node_data, user_data)) != IK_OK)
        FAIL(construct_failed, status);

    /* Since we malloc'd, we must also tell the refcounted object to free */
    (*node_data)->destroy = node_data_destroy;

    return IK_OK;

    construct_failed : FREE(*node_data);
    malloc_failed    : return status;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_data_construct(struct ik_node_data_t* node_data, const void* user_data)
{
    ikret_t status;
    if ((status = ik_refcounted_create(
            (struct ik_refcounted_t*)node_data,
            (ik_destroy_func)node_data_destruct)) != IK_OK)
        return status;

    memset(node_data, 0, sizeof *node_data);

    node_data->user_data = user_data;
    node_data->rotation_weight = 0.0;
    node_data->dist_to_parent = 0.0;
    node_data->mass = 0.0;
    ik_quat_set_identity(node_data->rotation.f);
    ik_vec3_set_zero(node_data->position.f);

    return IK_OK;
}
