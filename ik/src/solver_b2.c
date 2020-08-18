#include "ik/effector.h"
#include "ik/node.h"
#include "ik/log.h"
#include "ik/solver.h"
#include "ik/subtree.h"
#include "ik/transform.h"
#include "ik/quat.inl"
#include "ik/vec3.inl"
#include <assert.h>
#include <math.h>
#include <stddef.h>

struct ik_solver_b2
{
    IK_SOLVER_HEAD

    struct ik_node* base;
    struct ik_node* mid;
    struct ik_node* tip;
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
        ik_log_printf(IK_ERROR, "2B: Expected 1 end effector, but %d end effectors were found in the subtree. Use a more general algorithm (e.g. FABRIK)", subtree_leaves(subtree));
        return -1;
    }

    solver->tip = subtree_get_leaf(subtree, 0);
    solver->mid = solver->tip->parent;
    solver->base = solver->mid ? solver->mid->parent : NULL;

    if (solver->base == NULL)
    {
        ik_log_printf(IK_ERROR, "2B: Require exactly two bones, but only one bone was found in the subtree.");
        return -1;
    }
    if (solver->base != subtree->root)
    {
        ik_log_printf(IK_ERROR, "2B: Require exactly two bones, but a chain with more than 2 bones was found in the subtree.");
        return -1;
    }

    if (solver->algorithm->features & IK_ALGORITHM_CONSTRAINTS)
    {
        if (solver->tip->constraint == NULL && solver->mid->constraint == NULL)
        {
            ik_log_printf(IK_WARN, "2B: IK_ALGORITHM_CONSTRAINTS is set, but no constraints were found attached to the tip or mid nodes. Flag will be ignored.");
        }
    }

    IK_INCREF(solver->base);
    IK_INCREF(solver->mid);
    IK_INCREF(solver->tip);

    ik_log_printf(IK_DEBUG, "2B: Initialized");

    return 0;
}

/* ------------------------------------------------------------------------- */
static void
solver_b2_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    IK_DECREF(solver->tip);
    IK_DECREF(solver->mid);
    IK_DECREF(solver->base);
}

/* ------------------------------------------------------------------------- */
static int
solver_b2_solve(struct ik_solver* solver_base)
{
    ikreal a, b, c, aa, bb, cc;

    union ik_vec3 target_pos;
    union ik_quat alpha_rot;

    struct ik_solver_b2* s = (struct ik_solver_b2*)solver_base;
    struct ik_node* base = s->base;
    struct ik_node* mid = s->mid;
    struct ik_node* tip = s->tip;
    struct ik_effector* e = s->tip->effector;

    /* Tree and effector target position are in local space. Transform everything
     * into base node space */
    target_pos = e->target_position;
    ik_transform_pos_g2l(target_pos.f, tip, base);

    /*
     * Form a triangle from the two segment lengths so we can calculate the
     * angles. Here's some visual help.
     *
     *   target *--.__  a
     *           \     --.___ (unknown position, needs solving)
     *            \      _-
     *           c \   _-
     *              \-    b
     *            base
     *
     */
    a = s->tip->position.v.z;
    b = s->mid->position.v.z;
    aa = a*a;
    bb = b*b;
    cc = ik_vec3_length_squared(target_pos.f);
    c = sqrt(cc);

    /* check if in reach */
    if (c < a + b)
    {
        /* Cosine law to get base angle (alpha) */
        ikreal base_angle = acos((bb + cc - aa) / (2.0 * b * c));
        ikreal cos_a = cos(base_angle * 0.5);
        ikreal sin_a = sin(base_angle * 0.5);

        /*
         * The plane in which both segments lie is best determined by the
         * plane spanned by the pole vector and the target position. If no
         * pole vector exists, then we use the plane spanned by the mid position
         * and the target position. If these two vectors happen to be colinear
         * then fall back to using the tip position instead of the mid position.
         * If this too is colinear with the target vector then as a last restort,
         * simply use 0,0,1 as our rotation axis.
         */
        if (s->mid->pole)
            ik_vec3_copy(alpha_rot.f, s->mid->pole->position.f);
        else
            ik_vec3_copy(alpha_rot.f, mid->position.f);
        /*ik_transform_pos_l2g(alpha_rot.f, mid, base);*/
        ik_vec3_cross(alpha_rot.f, target_pos.f);

        /* if mid and target vectors are colinear... */
        if (!ik_vec3_normalize(alpha_rot.f))
        {
            ik_vec3_copy(alpha_rot.f, tip->position.f);
            /*ik_transform_pos_l2g(alpha_rot.f, tip, base);*/
            ik_vec3_cross(alpha_rot.f, target_pos.f);

            /* if tip and target vectors are also colinear... */
            if (!ik_vec3_normalize(alpha_rot.f))
                ik_vec3_set(alpha_rot.f, 0, 0, 1);
        }

        /*
         * Normalized cross product of alpha_rot gives us the axis of rotation.
         * Now we can scale the components and set the w component to construct
         * the quaternion describing the correct delta rotation of alpha.
         */
        ik_vec3_mul_scalar(alpha_rot.f, sin_a);
        alpha_rot.q.w = cos_a;

        ik_quat_mul_angle_of(alpha_rot.f, target_pos.f);
#if 0
        /*
         * Rotate side c and scale to length of side b to get the unknown
         * position. node_base was already subtracted from node_mid
         * previously, which means it will rotate around the base node's
         * position (as it should)
         */
        ik_vec3_copy(mid_pos, target_pos);
        ik_vec3_normalize(mid_pos);
        ik_vec3_rotate_quat(mid_pos, alpha_rot);
        ik_vec3_mul_scalar(mid_pos, b);
        /*ik_vec3_add_vec3(mid_pos, base_pos);*/

        ik_vec3_copy(tip_pos, target_pos);
#endif
    }
    else
    {
#if 0
        /* Just point both segments at target */
        ik_vec3_normalize(target_pos);
        ik_vec3_copy(mid_pos, target_pos);
        ik_vec3_copy(tip_pos, target_pos);
        ik_vec3_mul_scalar(mid_pos, b);
        ik_vec3_mul_scalar(tip_pos, a);
        /*ik_vec3_add_vec3(mid_pos, base_pos);*/
        ik_vec3_add_vec3(tip_pos, mid_pos);
#endif
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
static void
solver_b2_iterate_nodes(const struct ik_solver* solver_base, ik_solver_callback_func cb, int skip_base)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    if (!skip_base)
        cb(solver->base);

    cb(solver->mid);
    cb(solver->tip);
}

/* ------------------------------------------------------------------------- */
static void
solver_b2_iterate_effector_nodes(const struct ik_solver* solver_base, ik_solver_callback_func cb)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    cb(solver->tip);
}

/* ------------------------------------------------------------------------- */
static void
solver_b2_get_first_segment(const struct ik_solver* solver_base, struct ik_node** base, struct ik_node** tip)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    *base = solver->base;
    *tip = solver->mid;
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_TWO_BONE = {
    "two bone",
    sizeof(struct ik_solver_b2),
    solver_b2_init,
    solver_b2_deinit,
    solver_b2_solve,
    solver_b2_iterate_nodes,
    solver_b2_iterate_effector_nodes,
    solver_b2_get_first_segment
};
