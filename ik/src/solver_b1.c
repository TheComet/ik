#include "ik/effector.h"
#include "ik/node.h"
#include "ik/log.h"
#include "ik/solver.h"
#include "ik/subtree.h"
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
solver_b1_solve(struct ik_solver* solver_base)
{
    struct ik_solver_b1* solver = (struct ik_solver_b1*)solver_base;
    struct ik_effector* eff = solver->tip->effector;
    ikreal bone_length = solver->tip->dist_to_parent;
    ikreal* tip_pos = solver->tip->position.f;
    ikreal* base_pos = solver->base->position.f;
    ikreal* target_pos = eff->target_position.f;

    ik_vec3_copy(tip_pos, target_pos);
    ik_vec3_sub_vec3(tip_pos, base_pos);
    ik_vec3_normalize(tip_pos);
    ik_vec3_mul_scalar(tip_pos, bone_length);
    ik_vec3_add_vec3(tip_pos, base_pos);

    return 0;
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
