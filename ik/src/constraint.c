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
apply_dummy(struct ik_constraint* constraint, ikreal rotation[4])
{
}

/* ------------------------------------------------------------------------- */
static void
apply_stiff(struct ik_constraint* constraint, ikreal rotation[4])
{
    ik_quat_copy(rotation, constraint->data.stiff.rotation.f);
}

/* ------------------------------------------------------------------------- */
static void
apply_hinge(struct ik_constraint* constraint, ikreal rotation[4])
{
}

/* ------------------------------------------------------------------------- */
static void
apply_cone(struct ik_constraint* constraint, ikreal rotation[4])
{
    /* L2 distance between the constraint rotation and the node's rotation */
    ikreal dot = ik_quat_dot(constraint->data.cone.angle.f, rotation);
    dot = 2.0 * (1.0 - fabs(dot));
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
    struct ik_constraint* constraint = (struct ik_constraint*)
        ik_attachment_alloc(sizeof *constraint, (ik_deinit_func)deinit_constraint);
    if (constraint == NULL)
        return NULL;

    constraint->next = NULL;
    ik_constraint_set_custom(constraint, apply_dummy, NULL);

    return constraint;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_append(struct ik_constraint* first_constraint,
                     struct ik_constraint* constraint)
{
    while (first_constraint->next)
        first_constraint = first_constraint->next;
    first_constraint->next = constraint;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_set_stiff(struct ik_constraint* constraint,
                        ikreal qx, ikreal qy, ikreal qz, ikreal qw)
{
    constraint->apply = apply_stiff;
    ik_quat_set(constraint->data.stiff.rotation.f, qx, qy, qz, qw);
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_set_hinge(struct ik_constraint* constraint,
                        ikreal axis_x, ikreal axis_y, ikreal axis_z,
                        ikreal min_angle, ikreal max_angle)
{
    constraint->apply = apply_hinge;
    constraint->data.hinge.min_angle = min_angle;
    constraint->data.hinge.max_angle = max_angle;
    ik_vec3_set(constraint->data.hinge.axis.f, axis_x, axis_y, axis_z);
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
ik_constraint_set_custom(struct ik_constraint* constraint,
                         ik_constraint_apply_func callback,
                         void* data)
{
    constraint->apply = callback;
    constraint->data.custom.data = data;
}
