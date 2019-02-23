#include "ik/memory.h"
#include "ik/constraint.h"
#include "ik/log.h"
#include "ik/node_data.h"
#include "ik/quat.h"
#include <string.h>
#include <assert.h>

#define FAIL(errcode, label) do { \
        status = errcode; \
        goto label; \
    } while (0)

/* ------------------------------------------------------------------------- */
/* Constraint implementations */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static void
apply_dummy(const struct ik_node_data_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_set_identity(compensate_rotation);
}

/* ------------------------------------------------------------------------- */
static void
apply_stiff(const struct ik_node_data_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_copy(compensate_rotation, node->transform.t.rotation.f);
    ik_quat_conj(compensate_rotation);
    ik_quat_mul_quat(compensate_rotation, node->constraint->data.stiff.angle.f);
}

/* ------------------------------------------------------------------------- */
static void
apply_hinge(const struct ik_node_data_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_set_identity(compensate_rotation);
}

/* ------------------------------------------------------------------------- */
static void
apply_cone(const struct ik_node_data_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_set_identity(compensate_rotation);
}

/* ------------------------------------------------------------------------- */
/* Constraint API */
/* ------------------------------------------------------------------------- */

static void
destruct_constraint(struct ik_constraint_t* constraint)
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
        FAIL(IK_ERR_OUT_OF_MEMORY, alloc_constraint_failed);
    }

    memset(*constraint, 0, sizeof **constraint);

    if ((status = ik_refcount_create(&(*constraint)->refcount,
                  (ik_destruct_func)destruct_constraint, 1)) != IK_OK)
        FAIL(status, init_refcount_failed);

    ik_constraint_set_custom(*constraint, apply_dummy);

    return IK_OK;

    init_refcount_failed    : FREE(*constraint);
    alloc_constraint_failed : return status;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_destroy(struct ik_constraint_t* constraint)
{
    IK_DECREF(constraint);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_constraint_set_type(struct ik_constraint_t* constraint,
                       enum ik_constraint_type_e constraint_type)
{
    switch (constraint_type)
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

        case IK_CONSTRAINT_TYPES_COUNT : break;
    }

    constraint->type = constraint_type;
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_set_custom(struct ik_constraint_t* constraint, ik_constraint_apply_func callback)
{
    constraint->apply = callback;
}
