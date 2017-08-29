#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/solver.h"
#include "ik/solver_FABRIK.h"
#include "ik/solver_2bone.h"
#include "ik/solver_1bone.h"
#include "ik/solver_MSD.h"
#include "ik/solver_jacobian_inverse.h"
#include "ik/solver_jacobian_transpose.h"
#include "ik/transform.h"
#include <string.h>
#include <assert.h>

static int
recursively_get_all_effector_nodes(ik_node_t* node, vector_t* effector_nodes_list);

/* ------------------------------------------------------------------------- */
ik_solver_t*
ik_solver_create(enum solver_algorithm_e algorithm)
{
    int (*solver_construct)(ik_solver_t*) = NULL;
    ik_solver_t* solver = NULL;

    /*
     * Determine the correct size and confunction, depending on the
     * selected algorithm.
     */
    switch (algorithm)
    {
    case SOLVER_TWO_BONE:
        solver_construct = solver_2bone_construct;
        break;

    case SOLVER_ONE_BONE:
        solver_construct = solver_1bone_construct;
        break;

    case SOLVER_FABRIK:
        solver_construct = solver_FABRIK_construct;
        break;

    case SOLVER_MSD:
        solver_construct = solver_MSD_construct;
        break;

    /*
    case SOLVER_JACOBIAN_INVERSE:
    case SOLVER_JACOBIAN_TRANSPOSE:
        break;*/
    }

    if (solver_construct == NULL)
    {
        ik_log_message("Unknown algorithm \"%d\" was specified", algorithm);
        goto alloc_solver_failed;
    }

    /*
     * Allocate the solver, initialise to 0 and initialise the base fields
     * before calling the construct() callback for the specific solver.
     */
    solver = (ik_solver_t*)MALLOC(sizeof *solver);
    if (solver == NULL)
    {
        ik_log_message("Failed to allocate solver: ran out of memory");
        goto alloc_solver_failed;
    }
    memset(solver, 0, sizeof *solver);

    vector_construct(&solver->effector_nodes_list, sizeof(ik_node_t*));
    vector_construct(&solver->base_chain_list, sizeof(base_chain_t));

    /* Now call derived construction */
    if (solver_construct(solver) < 0)
        goto construct_derived_solver_failed;

    /* Derived destruct callback must be set */
    if (solver->destruct == NULL)
    {
        ik_log_message("Derived solvers MUST implement the destruct() callback");
        goto derived_didnt_implement_destruct;
    }

    return solver;

    derived_didnt_implement_destruct :
    construct_derived_solver_failed  : FREE(solver);
    alloc_solver_failed              : return NULL;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destroy(ik_solver_t* solver)
{
    solver->destruct(solver);

    if (solver->tree)
        ik_node_destroy(solver->tree);

    SOLVER_FOR_EACH_BASE_CHAIN(solver, base_chain)
        base_chain_destruct(base_chain);
    SOLVER_END_EACH
    vector_clear_free(&solver->base_chain_list);

    vector_clear_free(&solver->effector_nodes_list);

    FREE(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_set_tree(ik_solver_t* solver, ik_node_t* base)
{
    ik_solver_destroy_tree(solver);
    solver->tree = base;
}

/* ------------------------------------------------------------------------- */
ik_node_t*
ik_solver_unlink_tree(ik_solver_t* solver)
{
    ik_node_t* base = solver->tree;
    if (base == NULL)
        return NULL;
    solver->tree = NULL;

    /*
     * Effectors are owned by the nodes, but we need to release references to
     * them.
     */
    vector_clear(&solver->effector_nodes_list);

    return base;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destroy_tree(ik_solver_t* solver)
{
    ik_node_t* base;
    if ((base = ik_solver_unlink_tree(solver)) == NULL)
        return;
    ik_node_destroy(base);
}

/* ------------------------------------------------------------------------- */
int
ik_solver_rebuild_chain_trees(ik_solver_t* solver)
{
    /* If the solver has no tree, then there's nothing to do */
    if (solver->tree == NULL)
    {
        ik_log_message("No tree to work with. Did you forget to set the tree with ik_solver_set_tree()?");
        return -1;
    }

    /*
     * Traverse the entire tree and generate a list of the effectors. This
     * makes the process of building the chain list for FABRIK much easier.
     */
    ik_log_message("Rebuilding effector nodes list");
    vector_clear(&solver->effector_nodes_list);
    if (recursively_get_all_effector_nodes(
            solver->tree,
            &solver->effector_nodes_list) < 0)
    {
        ik_log_message("Ran out of memory while building the effector nodes list");
        return -1;
    }

    /* now build the chain tree */
    if (chain_tree_rebuild(
            &solver->base_chain_list,
            solver->tree,
            &solver->effector_nodes_list) < 0)
        return -1;

    if (solver->post_chain_build != NULL)
        return solver->post_chain_build(solver);

    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_recalculate_segment_lengths(ik_solver_t* solver)
{
    calculate_segment_lengths(&solver->base_chain_list);
}

/* ------------------------------------------------------------------------- */
int
ik_solver_solve(ik_solver_t* solver)
{
    int result;

    /*
     * All solvers work entirely, so we need to transform the tree first,
     * solve, then transform it back to local space. The algorithm requires
     * both the original and the active pose in local space.
     */
    ik_chains_global_to_local(&solver->base_chain_list, TRANSFORM_ORIGINAL | TRANSFORM_ACTIVE);

    if ((result = solver->solve(solver)) < 0)
        goto stop_and_return;

    if (solver->flags & SOLVER_CALCULATE_JOINT_ROTATIONS)
        ik_solver_calculate_joint_rotations(solver);

    stop_and_return: ik_chains_local_to_global(&solver->base_chain_list, TRANSFORM_ORIGINAL | TRANSFORM_ACTIVE);
    return result;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_calculate_joint_rotations(ik_solver_t* solver)
{
    SOLVER_FOR_EACH_BASE_CHAIN(solver, base_chain)
        calculate_global_rotations(base_chain);
    SOLVER_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
iterate_tree_recursive(ik_node_t* node,
                       ik_solver_iterate_node_cb_func callback)
{
    callback(node);

    NODE_FOR_EACH(node, guid, child)
        iterate_tree_recursive(child, callback);
    NODE_END_EACH
}
void
ik_solver_iterate_tree(ik_solver_t* solver,
                       ik_solver_iterate_node_cb_func callback)
{
    if (solver->tree == NULL)
    {
        ik_log_message("Warning: Tried iterating the tree, but no tree was set");
        return;
    }

    iterate_tree_recursive(solver->tree, callback);
}

/* ------------------------------------------------------------------------- */
static void
iterate_chain_tree_recursive(chain_t* chain,
                             ik_solver_iterate_node_cb_func callback)
{
    /*
     * Iterate the chain tree breadth first. Note that we exclude the base node
     * in each chain, because otherwise the same node would be passed to the
     * callback multiple times. The base node is shared by the parent chain's
     * effector as well as with other chains in the same depth.
     */
    int idx = chain_length(chain) - 1;
    assert(idx > 0); /* chains must have at least 2 nodes in them */
    while (idx--)
    {
        callback(chain_get_node(chain, idx));
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        iterate_chain_tree_recursive(child, callback);
    CHAIN_END_EACH
}
void
ik_solver_iterate_chain_tree(ik_solver_t* solver,
                             ik_solver_iterate_node_cb_func callback)
{
    SOLVER_FOR_EACH_BASE_CHAIN(solver, base_chain)
        iterate_chain_tree_recursive(base_chain, callback);
    SOLVER_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_solver_iterate_base_nodes(ik_solver_t* solver,
                             ik_solver_iterate_node_cb_func callback)
{
    SOLVER_FOR_EACH_BASE_CHAIN(solver, base_chain)
        assert(chain_length(base_chain) >= 2); /* chains should have at least 2 nodes */
        callback(chain_get_base_node(base_chain));
    SOLVER_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
reset_active_pose_recursive(ik_node_t* node)
{
    node->position = node->original_position;
    node->rotation = node->original_rotation;

    NODE_FOR_EACH(node, guid, child)
        reset_active_pose_recursive(child);
    NODE_END_EACH
}
void
ik_solver_reset_to_original_pose(ik_solver_t* solver)
{
    if (solver->tree == NULL)
        return;

    reset_active_pose_recursive(solver->tree);
}

/* ------------------------------------------------------------------------- */
static int
recursively_get_all_effector_nodes(ik_node_t* node, vector_t* effector_nodes_list)
{
    if (node->effector != NULL)
        if (vector_push(effector_nodes_list, &node) < 0)
            return -1;

    NODE_FOR_EACH(node, guid, child)
        if (recursively_get_all_effector_nodes(child, effector_nodes_list) < 0)
            return -1;
    NODE_END_EACH

    return 0;
}
