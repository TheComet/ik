#include "ik/ik.h"
#include "ik/memory.h"
#include "ik/impl/constraint_base.h"
#include "ik/impl/log.h"
#include "ik/impl/quat.h"
#include <string.h>
#include <assert.h>

static const struct ik_constraint_interface_t constraint_base = {
    IK_CONSTRAINT_BASE_IMPL
};

/* ------------------------------------------------------------------------- */
/* Constraint implementations */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static void
apply_dummy(const struct ik_node_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_set_identity(compensate_rotation);
}

/* ------------------------------------------------------------------------- */
static void
apply_stiff(const struct ik_node_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_set(compensate_rotation, node->rotation.f);
    ik_quat_conj(compensate_rotation);
    ik_quat_mul_quat(compensate_rotation, node->constraint->stiff.angle.f);
}

/* ------------------------------------------------------------------------- */
static void
apply_hinge(const struct ik_node_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_set_identity(compensate_rotation);
}

/* ------------------------------------------------------------------------- */
static void
apply_cone(const struct ik_node_t* node, ikreal_t compensate_rotation[4])
{
    ik_quat_set_identity(compensate_rotation);
}

/* ------------------------------------------------------------------------- */
/* Constraint API */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
struct ik_constraint_t*
ik_constraint_base_create()
{
    struct ik_constraint_t* constraint = MALLOC(sizeof *constraint);
    if (constraint == NULL)
    {
        ik_log_fatal("Failed to allocate constraint: Out of memory");
        return NULL;
    }

    memset(constraint, 0, sizeof *constraint);
    constraint->v = &constraint_base;
    ik_constraint_base_set_custom(constraint, apply_dummy);

    return constraint;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_base_destroy(struct ik_constraint_t* constraint)
{
    constraint->v->detach(constraint);
    FREE(constraint);
}

/* ------------------------------------------------------------------------- */
struct ik_constraint_t*
ik_constraint_base_duplicate(const struct ik_constraint_t* constraint)
{
    struct ik_constraint_t* new_constraint = constraint->v->create();
    if (new_constraint == NULL)
        return NULL;

    memcpy(new_constraint, constraint, sizeof *constraint);
    new_constraint->node = NULL;

    return new_constraint;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_constraint_base_set_type(struct ik_constraint_t* constraint, enum ik_constraint_type_e constraint_type)
{
    switch (constraint_type)
    {

        case IK_STIFF:
            constraint->apply = apply_stiff;
            break;

        case IK_HINGE:
            constraint->apply = apply_hinge;
            break;

        case IK_CONE:
            constraint->apply = apply_cone;
            break;

        case IK_CUSTOM:
            ik_log_error("Use constraint.set_custom() for type IK_CUSTOM. Constraint will have no effect.");
            return IK_WRONG_FUNCTION_FOR_CUSTOM_CONSTRAINT;
    }

    constraint->type = constraint_type;
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_base_set_custom(struct ik_constraint_t* constraint, ik_constraint_apply_func callback)
{
    constraint->apply = callback;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_base_detach(struct ik_constraint_t* constraint)
{
    if (constraint->node == NULL)
        return;

    constraint->node->constraint = NULL;
    constraint->node = NULL;
}

/* ------------------------------------------------------------------------- */
int
ik_constraint_base_attach(struct ik_constraint_t* constraint, struct ik_node_t* node)
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
    constraint->v->detach(constraint);

    constraint->node = node;
    node->constraint = constraint;

    return 0;
}
