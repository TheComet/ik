#include "ik/constraint.h"
#include "ik/log.h"
#include "ik/quat.inl"
#include "ik/tree_object.h"
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

/* ------------------------------------------------------------------------- */
static void
constraint_deinit(struct ik_constraint* constraint)
{
    IK_XDECREF(constraint->next);
}

/* ------------------------------------------------------------------------- */
struct ik_constraint*
ik_constraint_create(void)
{
    struct ik_constraint* constraint = (struct ik_constraint*)
        ik_attachment_alloc(sizeof *constraint, (ik_deinit_func)constraint_deinit);
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

    IK_INCREF(constraint);
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

/* ------------------------------------------------------------------------- */
static int
count_constraints_in_chain(const struct ik_constraint* constraint)
{
    int count;
    for (count = 0; constraint != NULL; constraint = constraint->next)
        count++;
    return count;
}
static int
count_constraints(const struct ik_tree_object* root)
{
    int count = count_constraints_in_chain(root->constraint);
    TREE_OBJECT_FOR_EACH_CHILD(root, child)
        count += count_constraints(child);
    TREE_OBJECT_END_EACH
    return count;
}

/* ------------------------------------------------------------------------- */
struct ik_constraint*
ik_constraint_duplicate(const struct ik_constraint* constraint)
{
    struct ik_constraint* dup = (struct ik_constraint*)
        ik_attachment_alloc(sizeof *dup, (ik_deinit_func)constraint_deinit);
    if (dup == NULL)
        return NULL;

    dup->next = NULL;
    dup->apply = constraint->apply;
    dup->data = constraint->data;

    return dup;
}

/* ------------------------------------------------------------------------- */
struct ik_constraint*
ik_constraint_duplicate_chain(const struct ik_constraint* constraint)
{
    int i;
    int count;
    struct ik_constraint* dup_buf;

    count = count_constraints_in_chain(constraint);
    assert(count > 0);

    dup_buf = (struct ik_constraint*)ik_refcounted_alloc_array(sizeof *dup_buf, (ik_deinit_func)constraint_deinit, count);
    if (dup_buf == NULL)
        return NULL;

    for (i = 0; i != count; ++i)
    {
        struct ik_constraint* dup = &dup_buf[i];

        dup->next = i < count-1 ? &dup_buf[i+1] : NULL;
        dup->apply = constraint->apply;
        dup->data = constraint->data;
        IK_XINCREF(dup->next);
    }

    return dup_buf;
}

/* ------------------------------------------------------------------------- */
static void
copy_from_tree(struct ik_constraint** con_buf,
               struct ik_tree_object* dst,
               const struct ik_tree_object* src)
{
    uint32_t i;
    struct ik_constraint* src_constraint;
    struct ik_constraint* first_in_chain;
    struct ik_constraint* old_constraint = NULL;

    if (src->constraint)
        first_in_chain = *con_buf;
    for (src_constraint = src->constraint; src_constraint != NULL; src_constraint = src_constraint->next)
    {
        struct ik_constraint* dst_constraint = *con_buf;
        (*con_buf)++;

        dst_constraint->next = NULL;
        dst_constraint->apply = src_constraint->apply;
        dst_constraint->data = src_constraint->data;

        if (old_constraint)
        {
            IK_INCREF(dst_constraint);
            old_constraint->next = dst_constraint;
        }
        old_constraint = dst_constraint;
    }

    if (src->constraint)
    {
        ik_tree_object_attach_constraint(dst, first_in_chain);
    }

    assert(ik_tree_object_child_count(src) == ik_tree_object_child_count(dst));
    for (i = 0; i != ik_tree_object_child_count(src); ++i)
    {
        copy_from_tree(con_buf,
                       ik_tree_object_get_child(dst, i),
                       ik_tree_object_get_child(src, i));
    }
}

/* ------------------------------------------------------------------------- */
int
ik_constraint_duplicate_from_tree(struct ik_tree_object* dst,
                                  const struct ik_tree_object* src)
{
    int count;
    struct ik_constraint* con_buf;

    count = count_constraints(src);
    if (count == 0)
        return 0;

    con_buf = (struct ik_constraint*)
        ik_refcounted_alloc_array(sizeof *con_buf, NULL, count);
    if (con_buf == NULL)
        return -1;

    copy_from_tree(&con_buf, dst, src);
    return 0;
}
