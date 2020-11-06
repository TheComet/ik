#include "ik/effector.h"
#include "ik/bone.h"
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

    struct ik_bone* bone;
};

/* ------------------------------------------------------------------------- */
static int
solver_b1_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;

    /* We need to assert that the subtree only contains a chain of length 1 */
    if (subtree_leaves(subtree) != 1)
    {
        ik_log_printf(IK_ERROR, "1-Bone: Expected 1 end effector, but %d end effectors were found in the subtree. Use a more general algorithm (e.g. FABRIK)", subtree_leaves(subtree));
        return -1;
    }

    solver->bone = subtree_get_root(subtree);

    if (solver->bone != subtree_get_leaf(subtree, 0))
    {
        ik_log_printf(IK_ERROR, "1-Bone: Require exactly one bone, but a chain with more than 1 bone was found in the subtree.");
        return -1;
    }

    if (solver->algorithm->features & IK_ALGORITHM_TARGET_ROTATIONS)
    {
        ik_log_printf(IK_WARN, "1-Bone: IK_ALGORITHM_TARGET_ROTATIONS is set, but target rotations are not supported. Flag will be ignored.");
    }

    if (solver->algorithm->features & IK_ALGORITHM_CONSTRAINTS)
    {
        if (solver->bone->constraint == NULL)
        {
            ik_log_printf(IK_WARN, "1-Bone: IK_ALGORITHM_CONSTRAINTS is set, but the tip bone does not have a constraint attached. Flag will be ignored.");
        }
    }

    /* Grab references to the bones we access later, in case nothing else
     * references them */
    IK_INCREF(solver->bone);

    ik_log_printf(IK_DEBUG, "1-Bone: Initialized");

    return 0;
}

/* ------------------------------------------------------------------------- */
static void
solver_b1_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;

    IK_DECREF(solver->bone);
}

/* ------------------------------------------------------------------------- */
static int
solver_b1_solve_no_constraints(struct ik_solver* solver_base)
{
    union ik_quat delta;
    union ik_vec3 target;
    struct ik_solver_b1* s = (struct ik_solver_b1*)solver_base;
    struct ik_bone* root = s->root_bone;
    struct ik_bone* bone = s->bone;
    struct ik_effector* e = bone->effector;

    /* Target position is in global space. Transform it into bone space */
    target = e->target_position;
    ik_transform_bone_pos_g2l(target.f, ik_bone_get_parent(root), bone);

    /*
     * Need to calculate the angle between where the bone is pointing, and the
     * target position. Because by convention the bone will always be Z-axis
     * aligned, we only have to calculate the angle of the target position.
     */
    ik_quat_angle_of(delta.f, target.f);
    ik_quat_mul_quat(bone->rotation.f, delta.f);

    return 1;
}

/* ------------------------------------------------------------------------- */
static int
solver_b1_solve_constraints(struct ik_solver* solver_base)
{
    union ik_quat delta;
    union ik_vec3 target;
    struct ik_solver_b1* s = (struct ik_solver_b1*)solver_base;
    struct ik_bone* root = s->root_bone;
    struct ik_bone* bone = s->bone;
    struct ik_effector* e = bone->effector;
    struct ik_constraint* c = bone->constraint;

    /* Target position is in global space. Transform it into bone space */
    target = e->target_position;
    ik_transform_bone_pos_g2l(target.f, ik_bone_get_parent(root), bone);

    /*
     * Need to calculate the angle between where the bone is pointing, and the
     * target position. Because by convention the bone will always be Z-axis
     * aligned, we only have to calculate the angle of the target position.
     */
    ik_quat_angle_of(delta.f, target.f);
    ik_quat_mul_quat(bone->rotation.f, delta.f);

    /* Apply constraint to base bone */
    assert(c);
    c->apply(c, bone->rotation.f);

    return 1;
}

/* ------------------------------------------------------------------------- */
static int
solver_b1_solve(struct ik_solver* solver_base)
{
    struct ik_solver_b1* s = (struct ik_solver_b1*)solver_base;

    if ((s->algorithm->features & IK_ALGORITHM_CONSTRAINTS) &&
        s->bone->constraint != NULL)
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
solver_b1_visit_bones(const struct ik_solver* solver_base, ik_bone_visit_func visit, void* param)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;

    visit(solver->bone, param);
}

/* ------------------------------------------------------------------------- */
static void
solver_b1_visit_effectors(const struct ik_solver* solver_base, ik_bone_visit_func visit, void* param)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;

    visit(solver->bone, param);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_ONE_BONE = {
    "one bone",
    sizeof(struct ik_solver_b1),
    solver_b1_init,
    solver_b1_deinit,
    solver_b1_solve,
    solver_b1_visit_bones,
    solver_b1_visit_effectors
};
