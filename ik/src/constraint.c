#include "ik/constraint.h"
#include "ik/log.h"
#include "ik/quat.h"
#include <string.h>
#include <assert.h>
#include <math.h>

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
    ik_quat_mul_quat_conj(delta_rotation, constraint->data.stiff.target_angle.f);
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
    /* L2 distance between the constraint rotation and the node's rotation */
    ikreal dot = ik_quat_dot(constraint->data.cone.angle.f, current_rotation);
    dot = 2.0 * (1.0 - fabs(dot));



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
void
ik_constraint_set_stiff(struct ik_constraint* constraint)
{
    constraint->apply = apply_stiff;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_set_hinge(struct ik_constraint* constraint,
                        ikreal axis_x, ikreal axis_y, ikreal axis_z,
                        ikreal min_angle, ikreal max_angle)
{
    constraint->apply = apply_hinge;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_set_cone(struct ik_constraint* constraint,
                       ikreal qx, ikreal qy, ikreal qz, ikreal qw,
                       ikreal min_angle, ikreal max_angle)
{
    constraint->apply = apply_cone;
    constraint->data.cone.min_angle = min_angle;
    constraint->data.cone.max_angle = max_angle;
    ik_quat_set(constraint->data.cone.angle.f, qx, qy, qz, qw);
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_set_custom(struct ik_constraint* constraint, ik_constraint_apply_func callback)
{
    constraint->apply = callback;
}
