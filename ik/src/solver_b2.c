#include "ik/effector.h"
#include "ik/node.h"
#include "ik/log.h"
#include "ik/solver.h"
#include "ik/subtree.h"
#include "ik/transform.h"
#include "ik/vec3.h"
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
        ik_log_printf(IK_ERROR, "This solver does not support multiple end effectors. You will need to switch to another solver (e.g. FABRIK)");
        return -1;
    }

    solver->tip = subtree_get_leaf(subtree, 0);
    solver->mid = solver->tip->parent;
    solver->base = solver->mid ? solver->mid->parent : NULL;

    if (solver->base == NULL || solver->base != subtree->root)
    {
        ik_log_printf(IK_ERROR, "Your tree has chains that are longer than 2 bones. The \"two bone\" solver only supports two bones.");
        return -1;
    }

    IK_INCREF(solver->base);
    IK_INCREF(solver->mid);
    IK_INCREF(solver->tip);

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
static void
solver_b2_update_translations(struct ik_solver* solver_base)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    solver->tip->dist_to_parent = ik_vec3_length(solver->tip->position.f);
    solver->mid->dist_to_parent = ik_vec3_length(solver->mid->position.f);
}

/* ------------------------------------------------------------------------- */
static int
solver_b2_solve(struct ik_solver* solver_base)
{
    ikreal a, b, c, aa, bb, cc;

    struct ik_solver_b2* s = (struct ik_solver_b2*)solver_base;

    struct ik_effector* e = s->tip->effector;
    ikreal* base_rot = s->base->rotation.f;
    ikreal* mid_pos = s->mid->position.f;
    /*ikreal* mid_rot = s->mid->rotation.f;*/
    ikreal* tip_pos = s->tip->position.f;
    /*ikreal* tip_rot = s->tip->rotation.f;*/
    ikreal* target_pos = e->target_position.f;

    /* Tree and effector target position are in local space. Transform everything
     * into base node space */
    ik_transform_pos_l2g(target_pos, s->tip, s->base);
    ik_transform_node_section_l2g(s->tip, s->base);

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
    a = s->tip->dist_to_parent;
    b = s->mid->dist_to_parent;
    aa = a*a;
    bb = b*b;
    cc = ik_vec3_length_squared(target_pos);
    c = sqrt(cc);

    /* check if in reach */
    if (c < a + b)
    {
        /* Cosine law to get base angle (alpha) */
        ikreal base_angle = acos((bb + cc - aa) / (2.0 * b * c));
        ikreal cos_a = cos(base_angle * 0.5);
        ikreal sin_a = sin(base_angle * 0.5);

        /* Cross product of both segment vectors defines axis of rotation */
        ik_vec3_copy(base_rot, tip_pos);
        ik_vec3_sub_vec3(base_rot, mid_pos);  /* top segment */
        /*ik_vec3_sub_vec3(mid_pos, base_pos);  * bottom segment */
        ik_vec3_cross(base_rot, mid_pos);

        /*
         * Set up quaternion describing the rotation of alpha. Need to
         * normalise vec3 component of quaternion so rotation is correct.
         * NOTE: Normalize will give us (1,0,0) in case of giving it a zero
         * length vector. We rely on this behavior for a default axis.
         */
        if (!ik_vec3_normalize(base_rot))
        {
            ik_vec3_copy(base_rot, target_pos);
            ik_vec3_cross(base_rot, tip_pos);
            ik_vec3_normalize(base_rot);
        }
        ik_vec3_mul_scalar(base_rot, sin_a);
        base_rot[3] = cos_a;  /* w component */

        /*
         * Rotate side c and scale to length of side b to get the unknown
         * position. node_base was already subtracted from node_mid
         * previously, which means it will rotate around the base node's
         * position (as it should)
         */
        ik_vec3_copy(mid_pos, target_pos);
        ik_vec3_normalize(mid_pos);
        ik_vec3_rotate_quat(mid_pos, base_rot);
        ik_vec3_mul_scalar(mid_pos, b);
        /*ik_vec3_add_vec3(mid_pos, base_pos);*/

        ik_vec3_copy(tip_pos, target_pos);
    }
    else
    {
        /* Just point both segments at target */
        ik_vec3_normalize(target_pos);
        ik_vec3_copy(mid_pos, target_pos);
        ik_vec3_copy(tip_pos, target_pos);
        ik_vec3_mul_scalar(mid_pos, b);
        ik_vec3_mul_scalar(tip_pos, a);
        /*ik_vec3_add_vec3(mid_pos, base_pos);*/
        ik_vec3_add_vec3(tip_pos, mid_pos);
    }

    /* Transform back to local space */
    ik_transform_node_section_g2l(s->tip, s->base);
    ik_transform_pos_g2l(target_pos, s->tip, s->base);

    return 0;
}

/* ------------------------------------------------------------------------- */
static void
solver_b2_iterate_nodes(const struct ik_solver* solver_base, ik_solver_callback_func cb)
{
    struct ik_solver_b2* solver = (struct ik_solver_b2*)solver_base;

    cb(solver->base);
    cb(solver->mid);
    cb(solver->tip);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_TWO_BONE = {
    "two bone",
    sizeof(struct ik_solver_b2),
    solver_b2_init,
    solver_b2_deinit,
    solver_b2_update_translations,
    solver_b2_solve,
    solver_b2_iterate_nodes
};
