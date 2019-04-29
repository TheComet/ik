#include "ik/ik.h"
#include "ik/memory.h"
#include "ik/node_data.h"
#include "ik/log.h"
#include "ik/pole.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include <stddef.h>
#include <string.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
/* Pole vector constraint implementations */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static void
calculate_roll_GENERIC(ikreal_t q[4], const struct ik_pole_t* pole)
{
    ik_quat_set_identity(q);
}

/* ------------------------------------------------------------------------- */
static void
calculate_roll_BLENDER(ikreal_t q[4], const struct ik_pole_t* pole)
{
    union ik_vec3_t work, x_axis, z_axis;

    /*
     * Determine "ik axis", which is the vector from the pole node to the
     * effector node.
     */
    ik_vec3_copy(work.f, pole->tip->transform.t.position.f);
    ik_vec3_sub_vec3(work.f, pole->node->transform.t.position.f);

    /*
     * Determine "pole axis", which is perpendicular to "ik axis" and points to
     * the pole position.
     */
    ik_vec3_project_from_vec3(work.f, pole->position.f);
    ik_vec3_sub_vec3(work.f, pole->position.f);

    /* Determine global XZ basis vectors of pole node */
    ik_vec3_set(x_axis.f, 1, 0, 0);
    ik_vec3_set(z_axis.f, 0, 0, 1);
    ik_vec3_rotate(x_axis.f, pole->node->transform.t.rotation.f);
    ik_vec3_rotate(z_axis.f, pole->node->transform.t.rotation.f);

    /* Project the pole axis onto these basis nodes to obtain the projected
     * pole axis */
    ik_vec3_normalize(work.f);
    ik_vec3_project_from_vec3_normalized(x_axis.f, work.f);
    ik_vec3_project_from_vec3_normalized(z_axis.f, work.f);
    ik_vec3_add_vec3(z_axis.f, x_axis.f);  /* z_axis is now projected pole axis */
}

/* ------------------------------------------------------------------------- */
static void
calculate_roll_MAYA(ikreal_t q[4], const struct ik_pole_t* pole)
{
    ik_quat_set_identity(q);
}

/* ------------------------------------------------------------------------- */
/* Pole API */
/* ------------------------------------------------------------------------- */

static void
deinit_pole(struct ik_pole_t* pole)
{
    /* No data is managed by pole */
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_pole_create(struct ik_pole_t** pole)
{
    ikret_t status;

    *pole = MALLOC(sizeof **pole);
    if (pole == NULL)
    {
        ik_log_fatal("Failed to allocate pole: Out of memory");
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, alloc_pole_failed);
    }

    memset(*pole, 0, sizeof **pole);

    if ((status = ik_refcount_create(&(*pole)->refcount,
            (ik_deinit_func)deinit_pole, 1)) != IK_OK)
        IK_FAIL(status, init_refcount_failed);

    (*pole)->angle = 0.0;
    (*pole)->calculate_roll = calculate_roll_GENERIC;
    ik_vec3_set_zero((*pole)->position.f);

    return IK_OK;

    init_refcount_failed : FREE(*pole);
    alloc_pole_failed    : return status;
}

/* ------------------------------------------------------------------------- */
void
ik_pole_free(struct ik_pole_t* pole)
{
    IK_DECREF(pole);
}

/* ------------------------------------------------------------------------- */
void
ik_pole_set_type(struct ik_pole_t* pole, enum ik_pole_type_e type)
{
    switch (type)
    {
#define X(arg) case IK_POLE_##arg : pole->calculate_roll = calculate_roll_##arg; break;
        IK_POLE_TYPE_LIST
#undef X

        default:
            ik_log_warning("ik_pole_set_type(): Unknown type %d, ignoring...", type);
            break;
    }
}
