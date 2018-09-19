#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/log.h"
#include "ik/pole.h"
#include "ik/solver.h"
#include "ik/solverdef.h"
#include "ik/solver_ONE_BONE.h"
#include "ik/solver_TWO_BONE.h"
#include "ik/solver_FABRIK.h"
#include "ik/solver_MSS.h"
#include <assert.h>
#include <string.h>

struct ik_solver_t
{
    SOLVER_HEAD
};

static int
recursively_get_all_effector_nodes(struct ik_node_t* node, struct vector_t* effector_nodes_list);

static void
determine_pole_target_tips(struct chain_t* chain);

static void
calculate_effector_target(const struct chain_t* chain);

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_create(struct ik_solver_t** solver, enum ik_solver_algorithm_e algorithm)
{
    switch (algorithm)
    {
#define X(algorithm)                                                          \
        case IK_SOLVER_##algorithm : {                                        \
            *solver = MALLOC(ik_solver_##algorithm##_type_size());            \
            if (*solver == NULL) {                                            \
                ik_log_fatal("Failed to allocate solver: ran out of memory"); \
                goto alloc_solver_failed;                                     \
            }                                                                 \
            memset(*solver, 0, ik_solver_##algorithm##_type_size());          \
            (*solver)->construct = ik_solver_##algorithm##_construct;         \
            (*solver)->destruct = ik_solver_##algorithm##_destruct;           \
            (*solver)->rebuild = ik_solver_##algorithm##_rebuild;             \
            (*solver)->solve = ik_solver_##algorithm##_solve;                 \
        } break;
        IK_SOLVER_ALGORITHM_LIST
#undef X
        default : {
            ik_log_error("Unknown solver algorithm with enum value %d", algorithm);
            goto alloc_solver_failed;
        } break;
    }

    if (ik_solver_construct(*solver) != IK_OK)
        goto construct_solver_failed;

    return IK_OK;

    construct_solver_failed : FREE(*solver);
    alloc_solver_failed     : return IK_ERR_OUT_OF_MEMORY;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destroy(struct ik_solver_t* solver)
{
    ik_solver_destruct(solver);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_construct(struct ik_solver_t* solver)
{
    solver->max_iterations = 20;
    solver->tolerance = 1e-2;
    solver->features = IK_SOLVER_JOINT_ROTATIONS;
    vector_construct(&solver->effector_nodes_list, sizeof(struct ik_node_t*));
    vector_construct(&solver->chain_list, sizeof(struct chain_t));

    return solver->construct(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destruct(struct ik_solver_t* solver)
{
    solver->destruct(solver);

    if (solver->tree)
        ik_node_destroy(solver->tree);

    SOLVER_FOR_EACH_CHAIN(solver, chain)
        chain_destruct(chain);
    SOLVER_END_EACH
    vector_clear_free(&solver->chain_list);

    vector_clear_free(&solver->effector_nodes_list);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_rebuild(struct ik_solver_t* solver)
{
    ikret_t result;

    /* If the solver has no tree, then there's nothing to do */
    if (solver->tree == NULL)
    {
        ik_log_error("No tree to work with. Did you forget to set the tree with ik_solver_set_tree()?");
        return IK_ERR_SOLVER_HAS_NO_TREE;
    }

    /*
     * Traverse the entire tree and generate a list of the effectors. This
     * makes the process of building the chain list for FABRIK much easier.
     */
    ik_log_info("Rebuilding effector nodes list");
    vector_clear(&solver->effector_nodes_list);
    if ((result = recursively_get_all_effector_nodes(
            solver->tree,
            &solver->effector_nodes_list)) != IK_OK)
    {
        ik_log_fatal("Ran out of memory while building the effector nodes list");
        return result;
    }

    /* now build the chain tree */
    if ((result = chain_tree_rebuild(
            &solver->chain_list,
            solver->tree,
            &solver->effector_nodes_list)) != IK_OK)
    {
        return result;
    }

    /* Pole targets need to know what their tip nodes are */
    VECTOR_FOR_EACH(&solver->chain_list, struct chain_t, chain)
        determine_pole_target_tips(chain);
    VECTOR_END_EACH

    update_distances(&solver->chain_list);

    return solver->rebuild(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_update_distances(struct ik_solver_t* solver)
{
    update_distances(&solver->chain_list);
}

/* ------------------------------------------------------------------------- */
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
ikret_t
ik_solver_solve(struct ik_solver_t* solver)
{
    update_actual_effector_targets(solver);
    return solver->solve(solver);
}

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_solver_get_tree(const struct ik_solver_t* solver)
{
    return solver->tree;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_set_tree(struct ik_solver_t* solver, struct ik_node_t* root)
{
    struct ik_node_t* old_root;
    if ((old_root = ik_solver_unlink_tree(solver)) != NULL)
        ik_node_destroy(old_root);

    solver->tree = root;
}

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_solver_unlink_tree(struct ik_solver_t* solver)
{
    struct ik_node_t* root = solver->tree;
    if (root == NULL)
        return NULL;
    solver->tree = NULL;

    /*
     * Effectors are owned by the nodes, but we need to release references to
     * them.
     */
    vector_clear(&solver->effector_nodes_list);

    return root;
}

/* ------------------------------------------------------------------------- */
uint32_t
ik_solver_get_max_iterations(const struct ik_solver_t* solver)
{
    return solver->max_iterations;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_set_max_iterations(struct ik_solver_t* solver, uint32_t max_iterations)
{
    solver->max_iterations = max_iterations;
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_solver_get_tolerance(const struct ik_solver_t* solver)
{
    return solver->tolerance;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_set_tolerance(struct ik_solver_t* solver, ikreal_t tolerance)
{
    solver->tolerance = tolerance;
}

/* ------------------------------------------------------------------------- */
uint8_t
ik_solver_get_features(const struct ik_solver_t* solver)
{
    return solver->features;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_set_features(struct ik_solver_t* solver, uint8_t features, int enabled)
{
    solver->features &= ~features;
    if (enabled)
        solver->features |= features;
}

/* ------------------------------------------------------------------------- */
static void
ik_solver_iterate_all_nodes_recursive(struct ik_node_t* node,
                                      ik_solver_iterate_node_cb_func callback)
{
    callback(node);

    NODE_FOR_EACH(node, guid, child)
        ik_solver_iterate_all_nodes_recursive(child, callback);
    NODE_END_EACH
}
void
ik_solver_iterate_all_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
{
    if (solver->tree == NULL)
    {
        ik_log_warning("Tried iterating the tree, but no tree was set");
        return;
    }

    ik_solver_iterate_all_nodes_recursive(solver->tree, callback);
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
ik_solver_iterate_affected_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
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
ik_solver_iterate_base_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
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

/* ------------------------------------------------------------------------- */
static void
determine_pole_target_tips(struct chain_t* chain)
{
    int idx;
    struct ik_node_t* last_tip_node;

    CHAIN_FOR_EACH_CHILD(chain, child)
        determine_pole_target_tips(child);
    CHAIN_END_EACH

    /* Want to start at tip and work our way down to the base of the chain */
    last_tip_node = chain_get_tip_node(chain);
    for (idx = 0; idx != (int)chain_length(chain); ++idx)
    {
        struct ik_node_t* node = chain_get_node(chain, idx);
        if (node->pole == NULL)
            continue;

        node->pole->tip = last_tip_node;
        last_tip_node = node;
    }
}

/* ------------------------------------------------------------------------- */
static void
calculate_effector_target(const struct chain_t* chain)
{
    /* Extract effector node and get its effector object */
    struct ik_node_t* node = chain_get_node(chain, 0);
    struct ik_effector_t* effector = node->effector;

    /* lerp using effector weight to get actual target position */
    effector->actual_target = effector->target_position;
    ik_vec3_sub_vec3(effector->actual_target.f, node->position.f);
    ik_vec3_mul_scalar(effector->actual_target.f, effector->weight);
    ik_vec3_add_vec3(effector->actual_target.f, node->position.f);

    /* Fancy algorithm using nlerp, makes transitions look more natural */
    if (effector->flags & IK_EFFECTOR_WEIGHT_NLERP && effector->weight < 1.0)
    {
        ikreal_t distance_to_target;
        struct ik_vec3_t base_to_effector;
        struct ik_vec3_t base_to_target;
        struct ik_node_t* base_node;

        /* Need distance from base node to target and base to effector node */
        base_node = chain_get_base_node(chain);
        base_to_effector = node->position;
        base_to_target = effector->target_position;
        ik_vec3_sub_vec3(base_to_effector.f, base_node->position.f);
        ik_vec3_sub_vec3(base_to_target.f, base_node->position.f);

        /* The effective distance is a lerp between these two distances */
        distance_to_target = ik_vec3_length(base_to_target.f) * effector->weight;
        distance_to_target += ik_vec3_length(base_to_effector.f) * (1.0 - effector->weight);

        /* nlerp the target position by pinning it to the base node */
        ik_vec3_sub_vec3(effector->actual_target.f, base_node->position.f);
        ik_vec3_normalize(effector->actual_target.f);
        ik_vec3_mul_scalar(effector->actual_target.f, distance_to_target);
        ik_vec3_add_vec3(effector->actual_target.f, base_node->position.f);
    }
}
