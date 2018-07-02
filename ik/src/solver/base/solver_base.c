#include "ik/ik.h"
#include "ik/solver_base.h"
#include "ik/chain.h"
#include "ik/memory.h"
#include "ik/quat_static.h"
#include "ik/transform.h"
#include "ik/vec3_static.h"
#include <string.h>
#include <assert.h>

static int
recursively_get_all_effector_nodes(struct ik_node_t* node, struct vector_t* effector_nodes_list);

/* ------------------------------------------------------------------------- */
uintptr_t
ik_solver_base_type_size(void)
{
    assert("Solver didn't override type_size()");
    return 0;
}

/* ------------------------------------------------------------------------- */
struct ik_solver_t*
ik_solver_base_create(enum ik_algorithm_e algorithm)
{
    assert("Don't use this function! Use ik.solver.create()");
    return NULL;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_base_destroy(struct ik_solver_t* solver)
{
    assert("Don't use this function! Use ik.solver.destroy()");
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_base_construct(struct ik_solver_t* solver)
{
    solver->max_iterations = 20;
    solver->tolerance = 1e-2;
    solver->flags = IK_ENABLE_JOINT_ROTATIONS;
    vector_construct(&solver->effector_nodes_list, sizeof(struct ik_node_t*));
    vector_construct(&solver->chain_list, sizeof(struct chain_t));
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_base_destruct(struct ik_solver_t* solver)
{
    if (solver->tree)
        solver->node->destroy(solver->tree);

    SOLVER_FOR_EACH_CHAIN(solver, chain)
        chain_destruct(chain);
    SOLVER_END_EACH
    vector_clear_free(&solver->chain_list);

    vector_clear_free(&solver->effector_nodes_list);
}

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_solver_base_unlink_tree(struct ik_solver_t* solver)
{
    struct ik_node_t* base = solver->tree;
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
ik_solver_base_destroy_tree(struct ik_solver_t* solver)
{
    struct ik_node_t* base;
    if ((base = solver->v->unlink_tree(solver)) == NULL)
        return;
    solver->node->destroy(base);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_base_set_tree(struct ik_solver_t* solver, struct ik_node_t* base)
{
    solver->v->destroy_tree(solver);
    solver->tree = base;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_base_rebuild(struct ik_solver_t* solver)
{
    ikret_t result;

    /* If the solver has no tree, then there's nothing to do */
    if (solver->tree == NULL)
    {
        IK.log.message("No tree to work with. Did you forget to set the tree with ik_solver_set_tree()?");
        return IK_SOLVER_HAS_NO_TREE;
    }

    /*
     * Traverse the entire tree and generate a list of the effectors. This
     * makes the process of building the chain list for FABRIK much easier.
     */
    IK.log.message("Rebuilding effector nodes list");
    vector_clear(&solver->effector_nodes_list);
    if ((result = recursively_get_all_effector_nodes(
            solver->tree,
            &solver->effector_nodes_list)) != IK_OK)
    {
        IK.log.message("Ran out of memory while building the effector nodes list");
        return result;
    }

    /* now build the chain tree */
    if ((result = chain_tree_rebuild(
            &solver->chain_list,
            solver->tree,
            &solver->effector_nodes_list)) != IK_OK)
        return result;

    update_distances(&solver->chain_list);

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_base_update_distances(struct ik_solver_t* solver)
{
    update_distances(&solver->chain_list);
}

/* ------------------------------------------------------------------------- */
static void
calculate_effector_target(const struct chain_t* chain)
{
    /* Extract effector node and get its effector object */
    struct ik_node_t* node = chain_get_node(chain, 0);
    struct ik_effector_t* effector = node->effector;

    /* lerp using effector weight to get actual target position */
    effector->_actual_target = effector->target_position;
    ik_vec3_static_sub_vec3(effector->_actual_target.f, node->position.f);
    ik_vec3_static_mul_scalar(effector->_actual_target.f, effector->weight);
    ik_vec3_static_add_vec3(effector->_actual_target.f, node->position.f);

    /* Fancy algorithm using nlerp, makes transitions look more natural */
    if (effector->flags & EFFECTOR_WEIGHT_NLERP && effector->weight < 1.0)
    {
        ikreal_t distance_to_target;
        ik_vec3_t base_to_effector;
        ik_vec3_t base_to_target;
        struct ik_node_t* base_node;

        /* Need distance from base node to target and base to effector node */
        base_node = chain_get_base_node(chain);
        base_to_effector = node->position;
        base_to_target = effector->target_position;
        ik_vec3_static_sub_vec3(base_to_effector.f, base_node->position.f);
        ik_vec3_static_sub_vec3(base_to_target.f, base_node->position.f);

        /* The effective distance is a lerp between these two distances */
        distance_to_target = ik_vec3_static_length(base_to_target.f) * effector->weight;
        distance_to_target += ik_vec3_static_length(base_to_effector.f) * (1.0 - effector->weight);

        /* nlerp the target position by pinning it to the base node */
        ik_vec3_static_sub_vec3(effector->_actual_target.f, base_node->position.f);
        ik_vec3_static_normalize(effector->_actual_target.f);
        ik_vec3_static_mul_scalar(effector->_actual_target.f, distance_to_target);
        ik_vec3_static_add_vec3(effector->_actual_target.f, base_node->position.f);
    }
}
static void
update_actual_effector_targets_for_chain_tree(const struct chain_t* chain)
{
    assert(chain_length(chain) > 1);
    struct ik_node_t* effector_node = chain_get_node(chain, 0);
    if (effector_node->effector != NULL)
    {
        calculate_effector_target(chain);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        update_actual_effector_targets_for_chain_tree(child);
    CHAIN_END_EACH
}
static void
update_actual_effector_targets(const struct ik_solver_t* solver)
{
    VECTOR_FOR_EACH(&solver->chain_list, struct chain_t, chain)
        update_actual_effector_targets_for_chain_tree(chain);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
int
ik_solver_base_solve(struct ik_solver_t* solver)
{
    update_actual_effector_targets(solver);
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static void
iterate_tree_recursive(struct ik_node_t* node,
                       ik_solver_iterate_node_cb_func callback)
{
    callback(node);

    NODE_FOR_EACH(node, guid, child)
        iterate_tree_recursive(child, callback);
    NODE_END_EACH
}
void
ik_solver_base_iterate_all_nodes(struct ik_solver_t* solver,
                                 ik_solver_iterate_node_cb_func callback)
{
    if (solver->tree == NULL)
    {
        IK.log.message("Warning: Tried iterating the tree, but no tree was set");
        return;
    }

    iterate_tree_recursive(solver->tree, callback);
}

/* ------------------------------------------------------------------------- */
static void
iterate_affected_nodes_recursive(struct chain_t* chain,
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
        iterate_affected_nodes_recursive(child, callback);
    CHAIN_END_EACH
}
void
ik_solver_base_iterate_affected_nodes(struct ik_solver_t* solver,
                             ik_solver_iterate_node_cb_func callback)
{
    SOLVER_FOR_EACH_CHAIN(solver, chain)
        int base_idx = chain_length(chain) - 1;
        assert(base_idx > 0);
        callback(chain_get_node(chain, base_idx));

        iterate_affected_nodes_recursive(chain, callback);
    SOLVER_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_solver_base_iterate_base_nodes(struct ik_solver_t* solver,
                             ik_solver_iterate_node_cb_func callback)
{
    SOLVER_FOR_EACH_CHAIN(solver, chain)
        assert(chain_length(chain) >= 2); /* chains should have at least 2 nodes */
        callback(chain_get_base_node(chain));
    SOLVER_END_EACH
}

/* ------------------------------------------------------------------------- */
static int
recursively_get_all_effector_nodes(struct ik_node_t* node, struct vector_t* effector_nodes_list)
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
