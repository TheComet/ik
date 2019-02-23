#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/node_data.h"
#include "ik/ntf.h"
#include "ik/pole.h"
#include "ik/solver.h"
#include "ik/solver_head.h"
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

static void
determine_pole_target_tips(struct ik_ntf_t* ntf);

static void
calculate_local_effector_target(const struct ik_node_data_t* chain);

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
            memset(*solver, 0, sizeof(ik_solver_##algorithm##_t));            \
            (*solver)->construct = ik_solver_##algorithm##_construct;         \
            (*solver)->destruct = ik_solver_##algorithm##_destruct;           \
            (*solver)->prepare = ik_solver_##algorithm##_prepare;             \
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
    ik_ntf_list_construct(&solver->ntf_list);

    return solver->construct(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destruct(struct ik_solver_t* solver)
{
    solver->destruct(solver);
    ik_ntf_list_clear(&solver->ntf_list);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_prepare(struct ik_solver_t* solver, struct ik_node_t* node)
{
    ikret_t result;

    /* Pole targets need to know what their tip nodes are */
    VECTOR_FOR_EACH(&solver->ntf_list, struct ik_ntf_t, ntf)
        determine_pole_target_tips(ntf);
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
        calculate_local_effector_target(chain);
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
uint32_t
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
determine_pole_target_tips(struct ik_ntf_t* ntf)
{
    uint32_t last_tip_node = 0;
    uint32_t nodes_left = ntf->node_count;

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
calculate_local_effector_target(const struct ik_node_data_t* chain)
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
