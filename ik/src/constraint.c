#include "ik/memory.h"
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
apply_dummy(const struct ik_node_data_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_set_identity(compensate_rotation);
}

/* ------------------------------------------------------------------------- */
static void
apply_stiff(const struct ik_node_data_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_copy(compensate_rotation, node->rotation.f);
    ik_quat_conj(compensate_rotation);
    ik_quat_mul_quat(compensate_rotation, node->constraint->stiff.angle.f);
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

/* ------------------------------------------------------------------------- */
ikret_t
ik_constraint_create(struct ik_constraint_t** constraint)
{
    *constraint = MALLOC(sizeof **constraint);
    if (*constraint == NULL)
    {
        ik_log_fatal("Failed to allocate constraint: Out of memory");
        return IK_ERR_OUT_OF_MEMORY;
    }

    memset(constraint, 0, sizeof **constraint);
    ik_constraint_set_custom(*constraint, apply_dummy);

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_destroy(struct ik_constraint_t* constraint)
{
    ik_constraint_detach(constraint);
    FREE(constraint);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_constraint_duplicate(struct ik_constraint_t** dst,
                        const struct ik_constraint_t* src)
{
    ikret_t status;
    if ((status = ik_constraint_create(dst)) != IK_OK)
        return status;

    memcpy(*dst, src, sizeof *src);
    (*dst)->node = NULL;

    return IK_OK;
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

/* ------------------------------------------------------------------------- */
void
ik_constraint_detach(struct ik_constraint_t* constraint)
{
    if (constraint->node == NULL)
        return;

    constraint->node->constraint = NULL;
    constraint->node = NULL;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_constraint_attach(struct ik_constraint_t* constraint, struct ik_node_t* node)
{
    if (node->constraint != NULL)
    {
        ik_log_error(
            "You are trying to attach a constraint to a node that "
            "already has a constraint attached to it. The new constraint will "
            "not be attached!"
        );
        return -1;
    }

    /* constraint may be attached to another node */
    ik_constraint_detach(constraint);

    constraint->node = node;
    node->constraint = constraint;

    return 0;
}
