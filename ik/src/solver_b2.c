#include "ik/effector.h"
#include "ik/bone.h"
#include "ik/log.h"
#include "ik/solver.h"
#include "ik/subtree.h"
#include "ik/transform.h"
#include "ik/quat.inl"
#include "ik/vec3.inl"
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

struct ik_solver_b2
{
    IK_SOLVER_HEAD

    struct ik_bone* base;
    struct ik_bone* tip;
};

/* ------------------------------------------------------------------------- */
static int
solver_b2_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    /*
     * We need to assert that there really are only chains of length 2 and no
     * sub chains.
     */
    if (subtree_leaves(subtree) != 1)
    {
        ik_log_printf(IK_ERROR, "2-Bone: Expected 1 end effector, but %d end effectors were found in the subtree. Use a more general algorithm (e.g. FABRIK)", subtree_leaves(subtree));
        return -1;
    }

    solver->tip = subtree_get_leaf(subtree, 0);
    solver->base = ik_bone_get_parent(solver->tip);

    if (solver->base == NULL)
    {
        ik_log_printf(IK_ERROR, "2-Bone: Require exactly two bones, but only one bone was found in the subtree.");
        return -1;
    }
    if (solver->base != subtree_get_root(subtree))
    {
        ik_log_printf(IK_ERROR, "2-Bone: Require exactly two bones, but a chain with more than 2 bones was found in the subtree.");
        return -1;
    }

    if (solver->algorithm->features & IK_ALGORITHM_CONSTRAINTS)
    {
        if (solver->base->constraint == NULL && solver->tip->constraint == NULL)
        {
            ik_log_printf(IK_WARN, "2-Bone: IK_ALGORITHM_CONSTRAINTS is set, but no constraints were found attached to the tip or base bones. Flag will be ignored.");
        }
    }

    if (solver->algorithm->features & IK_ALGORITHM_POLES)
    {
        if (solver->base->pole != NULL)
            ik_log_printf(IK_WARN, "2-Bone: Pole attached on base bone does nothing.");
        if (solver->tip->pole == NULL)
            ik_log_printf(IK_WARN, "2-Bone: IK_ALGORITHM_POLES is set, but no pole was found attached to the tip bone. Flag will be ignored.");
    }

    IK_INCREF(solver->base);
    IK_INCREF(solver->tip);

    ik_log_printf(IK_DEBUG, "2-Bone: Initialized");

    return 0;
}

/* ------------------------------------------------------------------------- */
static void
solver_b2_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    IK_DECREF(solver->tip);
    IK_DECREF(solver->base);
}

/* ------------------------------------------------------------------------- */
static int
solver_b2_solve(struct ik_solver* solver_base)
{
    ikreal aa, bb, cc, a, b, c;

    union ik_vec3 target_pos, base_pos_head, tip_pos_tail, tip_pos_head;
    union ik_quat base_offset_rot;

    struct ik_solver_b2* s = (struct ik_solver_b2*)solver_base;
    struct ik_bone* base = s->base;
    struct ik_bone* tip = s->tip;
    struct ik_effector* e = tip->effector;

    /* Target position is in global space. Transform it into base bone space */
    target_pos = e->target_position;
    ik_transform_bone_pos_g2l(target_pos.f, ik_bone_get_parent(s->root_bone), base);

    /* Set head positions in local space */
    ik_vec3_set(base_pos_head.f, 0, 0, base->length);
    ik_vec3_set(tip_pos_head.f, 0, 0, tip->length);

    /* transform tip bone positions into base bone space */
    ik_vec3_set_zero(tip_pos_tail.f);
    ik_transform_bone_pos_l2g(tip_pos_tail.f, tip, base);
    ik_transform_bone_pos_l2g(tip_pos_head.f, tip, base);

    /*
     * The child (tip) bone can have an offset position relative to the base
     * bone's head position, which introduces an offset rotation that has to be
     * compensated. Calculate this rotation now.
     *
     *                   o <- tip_pos_head
     *       tip bone -> |
     *                   o <- tip_pos_tail
     *                  .
     *                 .
     *                .
     *               o <- base_pos_head
     *  base bone -> |
     *               o
     *
     */
    ik_quat_angle_between(base_offset_rot.f, tip_pos_tail.f, base_pos_head.f);

    /*
     * Form a triangle from the two bones so we can calculate the angles.
     *
     *                o
     *                |\
     *                | \  a (tip)
     *                |  \
     *              c |   o
     *                |  /
     *                | / b (base)
     *                |/
     *                o
     */
    aa = tip->length * tip->length;
    bb = ik_vec3_length_squared(tip_pos_tail.f);
    cc = ik_vec3_length_squared(target_pos.f);

    a = sqrt(aa);
    b = sqrt(bb);
    c = sqrt(cc);

    /* check if in reach */
    if (c < a + b && c > fabs(a - b))
    {
        union ik_quat alpha_rot, delta;

        /* Cosine law to get base angle (alpha) */
        ikreal alpha = acos((bb + cc - aa) / (2.0 * b * c));
        ikreal gamma = M_PI - acos((aa + bb - cc) / (2.0 * a * b));

        /*
         * The plane in which both bones lie is best determined by the
         * plane spanned by the pole vector and the target position. If no
         * pole vector exists, then we use the plane spanned by the mid position
         * and the target position. If these two vectors happen to be colinear
         * then fall back to using the tip position instead of the mid position.
         * If this too is colinear with the target vector then as a last restort,
         * simply use 1,0,0 as our rotation axis.
         */
        if (s->tip->pole && s->algorithm->features & IK_ALGORITHM_POLES)
        {
            ik_vec3_copy(alpha_rot.f, s->tip->pole->position.f);
            /* pole vector is in global space, transform it into base bone space */
            ik_transform_bone_pos_g2l(alpha_rot.f, ik_bone_get_parent(s->root_bone), base);
        }
        else
            ik_vec3_copy(alpha_rot.f, tip_pos_tail.f);
        ik_vec3_cross(alpha_rot.f, target_pos.f);

        /* if mid and target vectors are colinear... */
        if (!ik_vec3_normalize(alpha_rot.f))
        {
            ik_vec3_copy(alpha_rot.f, tip_pos_head.f);
            ik_vec3_cross(alpha_rot.f, target_pos.f);

            /* if tip and target vectors are also colinear... */
            if (!ik_vec3_normalize(alpha_rot.f))
                ik_vec3_set(alpha_rot.f, 1, 0, 0);
        }

        /* Use same axis for tip bone rotation */
        ik_vec3_copy(tip->rotation.f, alpha_rot.f);

        /*
         * Normalized cross product of alpha_rot gives us the axis of rotation.
         * Now we can scale the components and set the w component to construct
         * the quaternion describing the correct delta rotation of alpha.
         */
        ik_vec3_mul_scalar(alpha_rot.f, sin(alpha * 0.5));
        alpha_rot.q.w = cos(alpha * 0.5);
        ik_vec3_mul_scalar(tip->rotation.f, sin(gamma * 0.5));
        tip->rotation.q.w = cos(gamma * 0.5);

        ik_quat_angle_of(delta.f, target_pos.f);
        ik_quat_mul_quat(base->rotation.f, delta.f);
        ik_quat_mul_quat_conj(base->rotation.f, alpha_rot.f);
        ik_quat_mul_quat(base->rotation.f, base_offset_rot.f);
        ik_quat_mul_quat_conj(tip->rotation.f, base_offset_rot.f);
    }
    else if (c > a + b)
    {
        /* Case where target is out of reach in the outer radius. just point
         * both segments at target */
        union ik_quat delta;
        ik_quat_angle_of(delta.f, target_pos.f);
        ik_quat_mul_quat(base->rotation.f, delta.f);
        ik_quat_mul_quat(base->rotation.f, base_offset_rot.f);
        ik_quat_copy_conj(tip->rotation.f, base_offset_rot.f);
    }
    else if (a < b)
    {
        /* Case where target is out of reach in an inner radius, and tip bone
         * is smaller than base bone. Point base bone at target, and rotate tip
         * 180 so it points inwards. */
        union ik_quat delta;
        union ik_vec3 axis;
        ik_quat_angle_of(delta.f, target_pos.f);
        ik_quat_mul_quat(base->rotation.f, delta.f);
        ik_quat_mul_quat(base->rotation.f, base_offset_rot.f);

        ik_vec3_copy(axis.f, base->rotation.f);
        ik_quat_set_axis_angle(tip->rotation.f, axis.v.x, axis.v.y, axis.v.z, M_PI);
        ik_quat_mul_quat_conj(tip->rotation.f, base_offset_rot.f);
    }
    else
    {
        /* Case where target is out of reach in an inner radius, and tip bone
         * is larger than base bone. Point base bone away from target, and
         * rotate tip 180 so it points inwards. */
        union ik_quat delta, one_eighty;
        union ik_vec3 axis;
        ik_quat_angle_of(delta.f, target_pos.f);
        ik_quat_mul_quat(base->rotation.f, delta.f);
        ik_quat_mul_quat(base->rotation.f, base_offset_rot.f);

        ik_quat_copy_conj(tip->rotation.f, base_offset_rot.f);
        ik_vec3_copy(axis.f, base->rotation.f);
        ik_quat_set_axis_angle(one_eighty.f, axis.v.x, axis.v.y, axis.v.z, M_PI);
        ik_quat_mul_quat(base->rotation.f, one_eighty.f);
        ik_quat_mul_quat(tip->rotation.f, one_eighty.f);
    }

    /* Apply constraints */
    if (s->algorithm->features & IK_ALGORITHM_CONSTRAINTS)
    {
        if (base->constraint)
            base->constraint->apply(base->constraint, base->rotation.f);
        if (tip->constraint)
            tip->constraint->apply(tip->constraint, tip->rotation.f);
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
static void
solver_b2_visit_bones(const struct ik_solver* solver_base, ik_visit_bone_func visit, void* param)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    visit(solver->base, param);
    visit(solver->tip, param);
}

/* ------------------------------------------------------------------------- */
static void
solver_b2_visit_effectors(const struct ik_solver* solver_base, ik_visit_bone_func visit, void* param)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    visit(solver->tip, param);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_TWO_BONE = {
    "two bone",
    sizeof(struct ik_solver_b2),
    solver_b2_init,
    solver_b2_deinit,
    solver_b2_solve,
    solver_b2_visit_bones,
    solver_b2_visit_effectors
};
