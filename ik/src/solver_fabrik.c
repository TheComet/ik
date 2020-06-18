#include "ik/chain_tree.h"
#include "ik/effector.h"
#include "ik/node.h"
#include "ik/solver.h"

struct ik_solver_fabrik
{
    IK_SOLVER_HEAD

    struct ik_chain chain_tree;
};

/* ------------------------------------------------------------------------- */
static int
fabrik_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    if (chain_tree_init(&solver->chain_tree) != 0)
        goto chain_tree_init_failed;
    if (chain_tree_build(&solver->chain_tree, subtree) != 0)
        goto build_chain_tree_failed;

    return 0;

    build_chain_tree_failed : chain_tree_deinit(&solver->chain_tree);
    chain_tree_init_failed  : return -1;
}

/* ------------------------------------------------------------------------- */
static void
fabrik_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    chain_tree_deinit(&solver->chain_tree);
}

/* ------------------------------------------------------------------------- */
static void
fabrik_update_translations(struct ik_solver* solver_base)
{
}

/* ------------------------------------------------------------------------- */
static int
fabrik_solve(struct ik_solver* solver_base)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
static void
fabrik_iterate_nodes_recursive(const struct ik_chain* chain, ik_solver_callback_func callback)
{
    unsigned idx;

    CHAIN_FOR_EACH_CHILD(chain, child)
        fabrik_iterate_nodes_recursive(child, callback);
    CHAIN_END_EACH

    for (idx = 0; idx < chain_length(chain) - 1; ++idx)
        callback(chain_get_node(chain, idx));
}
static void
fabrik_iterate_nodes(const struct ik_solver* solver_base, ik_solver_callback_func callback, int skip_base)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    fabrik_iterate_nodes_recursive(&solver->chain_tree, callback);
    if (!skip_base)
        callback(chain_get_base_node(&solver->chain_tree));
}

/* ------------------------------------------------------------------------- */
static void
fabrik_iterate_effector_nodes_recursive(const struct ik_chain* chain, ik_solver_callback_func callback)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        fabrik_iterate_effector_nodes_recursive(child, callback);
    CHAIN_END_EACH

    /* All leaves in chain tree have an effector */
    if (chain_child_count(chain) == 0)
        callback(chain_get_tip_node(chain));
}
static void
fabrik_iterate_effector_nodes(const struct ik_solver* solver_base, ik_solver_callback_func callback)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;
    fabrik_iterate_effector_nodes_recursive(&solver->chain_tree, callback);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_FABRIK = {
    "fabrik",
    sizeof(struct ik_solver_fabrik),
    fabrik_init,
    fabrik_deinit,
    fabrik_update_translations,
    fabrik_solve,
    fabrik_iterate_nodes,
    fabrik_iterate_effector_nodes
};
