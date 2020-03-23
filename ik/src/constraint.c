#include "ik/constraint.h"
#include "ik/log.h"
#include "ik/quat.h"
#include <string.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
/* Constraint implementations */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static void
apply_dummy(struct ik_constraint* constraint,
            ikreal delta_rotation[4],
            const ikreal current_rotation[4])
{
    ik_quat_set_identity(delta_rotation);
}

/* ------------------------------------------------------------------------- */
static void
apply_stiff(struct ik_constraint* constraint,
            ikreal delta_rotation[4],
            const ikreal current_rotation[4])
{
    ik_quat_copy(delta_rotation, current_rotation);
    ik_quat_conj(delta_rotation);
    ik_quat_mul_quat(delta_rotation, constraint->data.stiff.angle.f);
}

/* ------------------------------------------------------------------------- */
static void
apply_hinge(struct ik_constraint* constraint,
            ikreal delta_rotation[4],
            const ikreal current_rotation[4])
{
    ik_quat_set_identity(delta_rotation);
}

/* ------------------------------------------------------------------------- */
static void
apply_cone(struct ik_constraint* constraint,
           ikreal delta_rotation[4],
           const ikreal current_rotation[4])
{
    ik_quat_set_identity(delta_rotation);
}

/* ------------------------------------------------------------------------- */
/* Constraint API */
/* ------------------------------------------------------------------------- */

static void
deinit_constraint(struct ik_constraint* con)
{
    /* No data is managed by constraint */
}

/* ------------------------------------------------------------------------- */
struct ik_constraint*
ik_constraint_create(void)
{
    struct ik_constraint* con = (struct ik_constraint*)
        ik_attachment_alloc(sizeof *con, (ik_deinit_func)deinit_constraint);
    if (con == NULL)
        return NULL;

    ik_constraint_set_custom(con, apply_dummy);

    return con;
}

/* ------------------------------------------------------------------------- */
ikret
ik_constraint_set_type(struct ik_constraint* constraint,
                       enum ik_constraint_type type)
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
            ik_log_printf(IK_ERROR, "Use ik_constraint_set_custom() for type IK_CUSTOM. Constraint will have no effect.");
            return IK_ERR_WRONG_FUNCTION_FOR_CUSTOM_CONSTRAINT;

        default:
            ik_log_printf(IK_ERROR, "ik_constraint_set_type(): Unknown type %d, ignoring...", type);
            break;
    }

    constraint->type = type;
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_set_custom(struct ik_constraint* constraint, ik_constraint_apply_func callback)
{
    constraint->apply = callback;
}
