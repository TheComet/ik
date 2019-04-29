#include "cstructures/memory.h"
#include "ik/constraint.h"
#include "ik/log.h"
#include "ik/node_data.h"
#include "ik/quat.h"
#include <string.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
/* Constraint implementations */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static void
apply_dummy(ikreal_t delta_rotation[4],
            const ikreal_t current_rotation[4],
            struct ik_constraint_t* constraint)
{
    ik_quat_set_identity(delta_rotation);
}

/* ------------------------------------------------------------------------- */
static void
apply_stiff(ikreal_t delta_rotation[4],
            const ikreal_t current_rotation[4],
            struct ik_constraint_t* constraint)
{
    ik_quat_copy(delta_rotation, current_rotation);
    ik_quat_conj(delta_rotation);
    ik_quat_mul_quat(delta_rotation, constraint->data.stiff.angle.f);
}

/* ------------------------------------------------------------------------- */
static void
apply_hinge(ikreal_t delta_rotation[4],
            const ikreal_t current_rotation[4],
            struct ik_constraint_t* constraint)
{
    ik_quat_set_identity(delta_rotation);
}

/* ------------------------------------------------------------------------- */
static void
apply_cone(ikreal_t delta_rotation[4],
           const ikreal_t current_rotation[4],
           struct ik_constraint_t* constraint)
{
    ik_quat_set_identity(delta_rotation);
}

/* ------------------------------------------------------------------------- */
/* Constraint API */
/* ------------------------------------------------------------------------- */

static void
deinit_constraint(struct ik_constraint_t* constraint)
{
    /* No data is managed by constraint */
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_constraint_create(struct ik_constraint_t** constraint)
{
    ikret_t status;

    *constraint = MALLOC(sizeof **constraint);
    if (*constraint == NULL)
    {
        ik_log_fatal("Failed to allocate constraint: Out of memory");
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, alloc_constraint_failed);
    }

    memset(*constraint, 0, sizeof **constraint);

    if ((status = ik_refcount_create(&(*constraint)->refcount,
                  (ik_deinit_func)deinit_constraint, 1)) != IK_OK)
        IK_FAIL(status, init_refcount_failed);

    ik_constraint_set_custom(*constraint, apply_dummy);

    return IK_OK;

    init_refcount_failed    : FREE(*constraint);
    alloc_constraint_failed : return status;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_free(struct ik_constraint_t* constraint)
{
    IK_DECREF(constraint);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_constraint_set_type(struct ik_constraint_t* constraint,
                       enum ik_constraint_type_e type)
{
    switch (type)
    {

        case IK_CONSTRAINT_STIFF:
            constraint->apply = apply_stiff;
            break;

        case IK_CONSTRAINT_HINGE:
            constraint->apply = apply_hinge;
            break;

        case IK_CONSTRAINT_CONE:
            constraint->apply = apply_cone;
            break;

        case IK_CONSTRAINT_CUSTOM:
            ik_log_error("Use constraint.set_custom() for type IK_CUSTOM. Constraint will have no effect.");
            return IK_ERR_WRONG_FUNCTION_FOR_CUSTOM_CONSTRAINT;

        default:
            ik_log_warning("ik_constraint_set_type(): Unknown type %d, ignoring...", type);
            break;
    }

    constraint->type = type;
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_set_custom(struct ik_constraint_t* constraint, ik_constraint_apply_func callback)
{
    constraint->apply = callback;
}
