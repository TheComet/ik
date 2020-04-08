#include "ik/effector.h"
#include "ik/node.h"
#include "ik/log.h"
#include "ik/solver.h"
#include "ik/subtree.h"
#include "ik/transform.h"
#include "ik/vec3.h"
#include <stddef.h>
#include <assert.h>

struct ik_solver_b1
{
    IK_SOLVER_HEAD

    struct ik_node* base;
    struct ik_node* tip;
};

/* ------------------------------------------------------------------------- */
static int
solver_b1_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;

    /* We need to assert that the subtree only contains a chain of length 1 */
    if (subtree_leaves(subtree) != 1)
    {
        ik_log_printf(IK_ERROR, "This solver does not support multiple end effectors. You will need to switch to another solver (e.g. FABRIK)");
        return -1;
    }

    solver->tip = subtree_get_leaf(subtree, 0);
    solver->base = solver->tip->parent;

    if (solver->base == NULL || solver->base != subtree->root)
    {
        ik_log_printf(IK_ERROR, "Your tree has chains that are longer than 1 bone. The \"one bone\" solver only supports one bone.");
        return -1;
    }

    if (solver->algorithm->features & IK_ALGORITHM_TARGET_ROTATIONS)
    {
        ik_log_printf(IK_WARN, "\"one bone\" solver does not support target rotations. Flag will be ignored.");
    }

    IK_INCREF(solver->base);
    IK_INCREF(solver->tip);

    return 0;
}

/* ------------------------------------------------------------------------- */
static void
solver_b1_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;

    IK_DECREF(solver->tip);
    IK_DECREF(solver->base);
}

/* ------------------------------------------------------------------------- */
static void
solver_b1_update_translations(struct ik_solver* solver_base)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;

    solver->tip->dist_to_parent = ik_vec3_length(solver->tip->position.f);
}

/* ------------------------------------------------------------------------- */
static int
solver_b1_solve_no_joint_rotations(struct ik_solver* solver_base)
{
    struct ik_solver_b1* s = (struct ik_solver_b1*)solver_base;
    struct ik_node* base = s->base;
    struct ik_node* tip = s->tip;
    struct ik_effector* e = s->tip->effector;

    /*
     * Effector target position is in local space (tip node space) but we need
     * it relative to the base node.
     */
    ik_transform_pos_l2g(e->target_position.f, tip, base);

    if (e->features & IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION)
    {
        union ik_quat delta;
        ik_quat_angle(delta.f, e->target_position.f, tip->position.f);
        ik_quat_mul_quat(tip->rotation.f, delta.f);
    }

    /* Point tip node to target position */
    ik_vec3_copy(tip->position.f, e->target_position.f);
    ik_vec3_normalize(tip->position.f);
    ik_vec3_mul_scalar(tip->position.f, tip->dist_to_parent);

    /*
     * Transform effector target position back into local space. We
     * don't know if the user will update the target position between solver
     * calls so this is necessary
     */
    ik_transform_pos_g2l(e->target_position.f, tip, base);

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
solver_b1_solve_joint_rotations(struct ik_solver* solver_base)
{
    union ik_quat delta;
    ikreal target_distance;
    struct ik_solver_b1* s = (struct ik_solver_b1*)solver_base;
    struct ik_node* base = s->base;
    struct ik_node* tip = s->tip;
    struct ik_effector* e = s->tip->effector;

    ikreal* target_pos = e->target_position.f;

    /*
     * Effector target position is in local space (tip node space) but we need
     * it relative to the base node.
     */
    ik_transform_pos_l2g(target_pos, tip, base);

    /*
     * Rotating the target position and then rotating the base node in the
     * opposite direction has the same effect as pointing the tip node
     * to the target position and then calculating the base node angle.
     */
    ik_quat_angle(delta.f, tip->position.f, target_pos);
    ik_quat_mul_quat(base->rotation.f, delta.f);

    target_distance = ik_vec3_length(target_pos);
    ik_vec3_copy(target_pos, tip->position.f);
    ik_vec3_normalize(target_pos);
    ik_vec3_mul_scalar(target_pos, target_distance);

    if (e->features & IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION)
        ik_quat_mul_quat_conj(tip->rotation.f, delta.f);

    /*
     * Transform effector target position back into local space. We
     * don't know if the user will update the target position between solver
     * calls so this is necessary
     */
    ik_transform_pos_g2l(target_pos, tip, base);

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
solver_b1_solve(struct ik_solver* solver_base)
{
    struct ik_solver_b1* s = (struct ik_solver_b1*)solver_base;
    const struct ik_algorithm* a = solver_base->algorithm;

    if (a->features & IK_ALGORITHM_JOINT_ROTATIONS)
        s->impl.solve = solver_b1_solve_joint_rotations;
    else
        s->impl.solve = solver_b1_solve_no_joint_rotations;

    return s->impl.solve(solver_base);
}

/* ------------------------------------------------------------------------- */
static void
solver_b1_iterate_nodes(const struct ik_solver* solver_base, ik_solver_callback_func cb)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;
    cb(solver->base);
    cb(solver->tip);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_ONE_BONE = {
    "one bone",
    sizeof(struct ik_solver_b1),
    solver_b1_init,
    solver_b1_deinit,
    solver_b1_update_translations,
    solver_b1_solve,
    solver_b1_iterate_nodes
};
