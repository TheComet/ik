#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/solver.h"
#include "ik/solver_FABRIK.h"
#include "ik/solver_jacobian_inverse.h"
#include "ik/solver_jacobian_transpose.h"
#include <string.h>

static int
recursively_get_all_effector_nodes(struct ik_node_t* node, struct ordered_vector_t* effector_nodes_list);

/* ------------------------------------------------------------------------- */
struct ik_solver_t*
ik_solver_create(enum solver_algorithm_e algorithm)
{
    uintptr_t solver_size = 0;
    int (*solver_construct)(struct ik_solver_t*) = NULL;
    struct ik_solver_t* solver = NULL;

    /*
     * Determine the correct size and construct function, depending on the
     * selected algorithm.
     */
    switch (algorithm)
    {
    case SOLVER_FABRIK:
        solver_size = sizeof(struct fabrik_t);
        solver_construct = solver_FABRIK_construct;
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
    solver = (struct ik_solver_t*)MALLOC(solver_size);
    if (solver == NULL)
    {
        ik_log_message("Failed to allocate solver: ran out of memory");
        goto alloc_solver_failed;
    }
    memset(solver, 0, solver_size);

    ordered_vector_construct(&solver->effector_nodes_list, sizeof(struct ik_node_t*));

    /* Use a chain to hold all of the disjoint chain trees */
    solver->chain_tree = chain_create();
    if (solver->chain_tree == NULL)
        goto alloc_chain_tree_failed;

    /* Now call derived construction */
    if (solver_construct(solver) < 0)
        goto construct_derived_solver_failed;

    /* Derived destruct function must be set */
    if (solver->destruct == NULL)
    {
        ik_log_message("Derived solvers MUST implement the destruct() callback");
        goto derived_didnt_implement_destruct;
    }

    return solver;

    derived_didnt_implement_destruct :
    construct_derived_solver_failed  : chain_destroy(solver->chain_tree);
    alloc_chain_tree_failed          : FREE(solver);
    alloc_solver_failed              : return NULL;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destroy(struct ik_solver_t* solver)
{
    solver->destruct(solver);

    if(solver->tree)
        ik_node_destroy(solver->tree);

    chain_destroy(solver->chain_tree);
    ordered_vector_clear_free(&solver->effector_nodes_list);

    FREE(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_set_tree(struct ik_solver_t* solver, struct ik_node_t* root)
{
    ik_solver_destroy_tree(solver);
    solver->tree = root;
}

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_solver_unlink_tree(struct ik_solver_t* solver)
{
    struct ik_node_t* root = solver->tree;
    if(root == NULL)
        return NULL;
    solver->tree = NULL;

    /*
     * Effectors are owned by the nodes, but we need to release references to
     * them.
     */
    ordered_vector_clear(&solver->effector_nodes_list);

    return root;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destroy_tree(struct ik_solver_t* solver)
{
    struct ik_node_t* root;
    if((root = ik_solver_unlink_tree(solver)) == NULL)
        return;
    ik_node_destroy(root);
}

/* ------------------------------------------------------------------------- */
int
ik_solver_rebuild_data(struct ik_solver_t* solver)
{
    /* If the solver has no tree, then there's nothing to do */
    if(solver->tree == NULL)
    {
        ik_log_message("No tree to work with. Did you forget to set the tree with ik_solver_set_tree()?");
        return -1;
    }

    /*
     * Traverse the entire tree and generate a list of the effectors. This
     * makes the process of building the chain list for FABRIK much easier.
     */
    ik_log_message("Rebuilding effector nodes list");
    ordered_vector_clear(&solver->effector_nodes_list);
    if (recursively_get_all_effector_nodes(solver->tree, &solver->effector_nodes_list) < 0)
    {
        ik_log_message("Ran out of memory while building the effector nodes list");
        return -1;
    }

    /* now build the chain tree */
    if (rebuild_chain_tree(solver) < 0)
        return -1;

    if (solver->rebuild_data != NULL)
        return solver->rebuild_data(solver);

    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_recalculate_segment_lengths(struct ik_solver_t* solver)
{
    calculate_segment_lengths(solver->chain_tree);
}

/* ------------------------------------------------------------------------- */
int
ik_solver_solve(struct ik_solver_t* solver)
{
    return solver->solve(solver);
}

/* ------------------------------------------------------------------------- */
static void
iterate_tree_recursive(struct ik_node_t* node,
                       ik_solver_iterate_node_cb_func callback)
{
    callback(node);

    BSTV_FOR_EACH(&node->children, struct ik_node_t, guid, child)
        iterate_tree_recursive(child, callback);
    BSTV_END_EACH
}
void
ik_solver_iterate_tree(struct ik_solver_t* solver,
                       ik_solver_iterate_node_cb_func callback)
{
    if(solver->tree == NULL)
    {
        ik_log_message("Warning: Tried iterating the tree, but no tree was set");
        return;
    }

    iterate_tree_recursive(solver->tree, callback);
}

/* ------------------------------------------------------------------------- */
static void
reset_solved_data_recursive(struct ik_node_t* node)
{
    node->position = node->initial_position;
    node->rotation = node->initial_rotation;

    BSTV_FOR_EACH(&node->children, struct ik_node_t, guid, child)
        reset_solved_data_recursive(child);
    BSTV_END_EACH
}
void
ik_solver_reset_solved_data(struct ik_solver_t* solver)
{
    if(solver->tree == NULL)
        return;

    reset_solved_data_recursive(solver->tree);
}

/* ------------------------------------------------------------------------- */
static int
recursively_get_all_effector_nodes(struct ik_node_t* node, struct ordered_vector_t* effector_nodes_list)
{
    if(node->effector != NULL)
        if(ordered_vector_push(effector_nodes_list, &node) < 0)
            return -1;

    BSTV_FOR_EACH(&node->children, struct ik_node_t, guid, child)
        if(recursively_get_all_effector_nodes(child, effector_nodes_list) < 0)
            return -1;
    BSTV_END_EACH

    return 0;
}
