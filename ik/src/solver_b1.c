#include "ik/effector.h"
#include "ik/node.h"
#include "ik/log.h"
#include "ik/solver.h"
#include "ik/subtree.h"
#include "ik/transform.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"
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
        ik_log_printf(IK_ERROR, "1B: Expected 1 end effector, but %d end effectors were found in the subtree. Use a more general algorithm (e.g. FABRIK)", subtree_leaves(subtree));
        return -1;
    }

    solver->tip = subtree_get_leaf(subtree, 0);
    solver->base = solver->tip->parent;

    if (solver->base == NULL)
    {
        ik_log_printf(IK_ERROR, "1B: Require exactly one bone, but zero were found in the subtree.");
        return -1;
    }
    if (solver->base != subtree->root)
    {
        ik_log_printf(IK_ERROR, "1B: Require exactly one bone, but a chain with more than 1 bone was found in the subtree.");
        return -1;
    }

    if (solver->algorithm->features & IK_ALGORITHM_TARGET_ROTATIONS)
    {
        ik_log_printf(IK_WARN, "1B: IK_ALGORITHM_TARGET_ROTATIONS is set, but target rotations are not supported. Flag will be ignored.");
    }

    if (solver->algorithm->features & IK_ALGORITHM_CONSTRAINTS)
    {
        if (solver->tip->constraint == NULL)
        {
            ik_log_printf(IK_WARN, "1B: IK_ALGORITHM_CONSTRAINTS is set, but the tip node does not have a constraint attached. Flag will be ignored.");
        }
    }

    /* Grab references to the nodes we access later, in case nothing else
     * references them */
    IK_INCREF(solver->base);
    IK_INCREF(solver->tip);

    ik_log_printf(IK_DEBUG, "1B: Initialized");

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
static int
solver_b1_solve_no_constraints(struct ik_solver* solver_base)
{
    union ik_quat delta;
    union ik_vec3 target;
    struct ik_solver_b1* s = (struct ik_solver_b1*)solver_base;
    struct ik_node* root = s->root_node;
    struct ik_node* base = s->base;
    struct ik_node* tip = s->tip;
    struct ik_effector* e = s->tip->effector;

    /* Transform target into base-node space */
    target = e->target_position;
    ik_transform_pos_g2l(target.f, root->parent, base);

    /*
     * Need to calculate the angle between where the bone is pointing, and the
     * target position. Because by convention the bone will always be Z-axis
     * aligned, we only have to calculate the angle of the target position.
     */
    ik_quat_angle_of(delta.f, target.f);
    ik_quat_mul_quat(base->rotation.f, delta.f);

    /* Rotate tip node with same rotation so it keeps its global orientation */
    if (e->features & IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION)
        ik_quat_mul_quat_conj(tip->rotation.f, delta.f);

    return 1;
}

/* ------------------------------------------------------------------------- */
static int
solver_b1_solve_constraints(struct ik_solver* solver_base)
{
    union ik_quat delta;
    union ik_quat constraint_delta;
    union ik_vec3 target;
    struct ik_solver_b1* s = (struct ik_solver_b1*)solver_base;
    struct ik_node* root = s->root_node;
    struct ik_node* base = s->base;
    struct ik_node* tip = s->tip;
    struct ik_effector* e = s->tip->effector;
    struct ik_constraint* c = tip->constraint;

    /* Transform target into base-node space */
    target = e->target_position;
    ik_transform_pos_g2l(target.f, root, base);

    /*
     * Need to calculate the angle between where the bone is pointing, and the
     * target position. Because by convention the bone will always be Z-axis
     * aligned, we only have to calculate the angle of the target position.
     */
    ik_quat_angle_of(delta.f, target.f);
    ik_quat_mul_quat(base->rotation.f, delta.f);

    /* Apply constraint to base node */
    assert(c);
    c->apply(c, base->rotation.f);

    /* Rotate tip node with same rotation so it keeps its global orientation */
    if (e->features & IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION)
    {
        ik_quat_mul_quat_conj(tip->rotation.f, delta.f);
        ik_quat_mul_quat_conj(delta.f, constraint_delta.f);
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
static int
solver_b1_solve(struct ik_solver* solver_base)
{
    struct ik_solver_b1* s = (struct ik_solver_b1*)solver_base;

    if ((s->algorithm->features & IK_ALGORITHM_CONSTRAINTS) &&
        s->tip->constraint != NULL)
    {
        s->impl.solve = solver_b1_solve_constraints;
    }
    else
    {
        s->impl.solve = solver_b1_solve_no_constraints;
    }

    return s->impl.solve(solver_base);
}

/* ------------------------------------------------------------------------- */
static void
solver_b1_visit_nodes(const struct ik_solver* solver_base, ik_visit_node_func visit, void* param, int skip_base)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;
    if (!skip_base)
        visit(solver->base, param);
    visit(solver->tip, param);
}

/* ------------------------------------------------------------------------- */
static void
solver_b1_visit_effector_nodes(const struct ik_solver* solver_base, ik_visit_node_func visit, void* param)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;

    visit(solver->tip, param);
}
/* ------------------------------------------------------------------------- */
static void
solver_b1_get_first_segment(const struct ik_solver* solver_base, struct ik_node** base, struct ik_node** tip)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;

    *base = solver->base;
    *tip = solver->tip;
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_ONE_BONE = {
    "one bone",
    sizeof(struct ik_solver_b1),
    solver_b1_init,
    solver_b1_deinit,
    solver_b1_solve,
    solver_b1_visit_nodes,
    solver_b1_visit_effector_nodes,
    solver_b1_get_first_segment
};
