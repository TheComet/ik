#include "ik/constraint_base.h"
#include "ik/ik.h"
#include "ik/memory.h"
#include <string.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
/* Constraint implementations */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static int
apply_none(struct ik_node_t* node)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
apply_stiff(struct ik_node_t* node)
{
    /*
     * The stiff constraint should never actually be reached, because joints
     * that have a stiff constraint will be excluded from the chain tree
     * entirely. This function exists solely to debug the chain tree.
     */
    assert(1);
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
apply_hinge(struct ik_node_t* node)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
apply_cone(struct ik_node_t* node)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Constraint API */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
void
ik_constraint_base_set(struct ik_constraint_t* constraint, ik_constraint_type_e constraint_type)
{
    switch (constraint_type)
    {
        case IK_CONSTRAINT_NONE:
            constraint->apply = apply_none;
            break;

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
            ik.log.message("Error: use constraint.set_custom() for type IK_CONSTRAINT_CUSTOM. Constraint will have no effect.");
    }

    constraint->type = constraint_type;
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
        ik.log.message(
            "Warning! You are trying to attach a constraint to a node that "
            "already has a constraint attached to it. The new constraint will "
            "not be attached!"
        );
        return -1;
    }

    /* constraint may be attached to another node */
    ik_constraint_base_detach(constraint);

    constraint->node = node;
    node->constraint = constraint;

    return 0;
}

/* ------------------------------------------------------------------------- */
struct ik_constraint_t*
ik_constraint_base_create(ik_constraint_type_e constraint_type)
{
    struct ik_constraint_t* constraint = MALLOC(sizeof *constraint);
    if (constraint == NULL)
    {
        ik.log.message("Failed to allocate constraint: Out of memory");
        return NULL;
    }

    memset(constraint, 0, sizeof *constraint);
    ik_constraint_base_set(constraint, constraint_type);
    constraint->v = &ik.internal.constraint_base;

    return constraint;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_base_destroy(struct ik_constraint_t* constraint)
{
    ik_constraint_base_detach(constraint);
    FREE(constraint);
}
