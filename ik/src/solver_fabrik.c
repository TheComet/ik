#include "ik/chain_tree.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/solver.h"
#include "ik/subtree.h"
#include "ik/transform.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"

#include "cstructures/memory.h"

#include <stddef.h>
#include <math.h>

struct ik_solver_fabrik
{
    IK_SOLVER_HEAD

    struct ik_chain chain_tree;
    struct ik_node** effector_nodes;
    union ik_quat* intermediate_rotations;
    union ik_vec3* target_positions;

    int num_effectors;
    int num_intermediate_rotations;
};

/* ------------------------------------------------------------------------- */
static int
validate_poles_recursive(const struct ik_solver_fabrik* solver, const struct ik_chain* chain)
{
    int poles_found = 0;
    CHAIN_FOR_EACH_CHILD(chain, child)
        poles_found += validate_poles_recursive(solver, child);
    CHAIN_END_EACH

    /* Pole target constraints should only be attached to the tip node of each
     * chain. */
    CHAIN_FOR_EACH_NODE_RANGE(chain, node, 0, chain_node_count(chain) - 1)
        if (node == chain_get_tip_node(chain))
            continue;
        if (node->pole != NULL)
        {
            ik_log_printf(IK_WARN, "(\"%s\" algorithm): Pole attached to node (ptr: 0x%p, guid: %d) has no effect and will be ignored.", solver->impl.name, node->user.ptr, node->user.guid);
            poles_found++;
        }
    CHAIN_END_EACH

    return poles_found;
}
static void
validate_poles(const struct ik_solver_fabrik* solver)
{
    if (validate_poles_recursive(solver, &solver->chain_tree))
    {
        ik_log_printf(IK_WARN, "(\"%s\" algorithm): Poles only make sense when attached to the end of chains, such as effector nodes, or nodes with multiple children.", solver->impl.name);
    }
}

/* ------------------------------------------------------------------------- */
static void
store_effector_nodes_recursive(struct ik_node*** effector_nodes_store, const struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        store_effector_nodes_recursive(effector_nodes_store, child);
    CHAIN_END_EACH

    if (chain_child_count(chain) == 0)
    {
        **effector_nodes_store = chain_get_tip_node(chain);
        (*effector_nodes_store)++;
    }
}
static void
store_effector_nodes(struct ik_solver_fabrik* solver)
{
    struct ik_node** effector_nodes_store = solver->effector_nodes;
    store_effector_nodes_recursive(&effector_nodes_store, &solver->chain_tree);

    /* sanity check */
    assert(effector_nodes_store - solver->effector_nodes == solver->num_effectors);
}

/* ------------------------------------------------------------------------- */
static void
update_target_data(struct ik_solver_fabrik* solver)
{
    int i;
    const struct ik_node* root = chain_get_base_node(&solver->chain_tree);

    for (i = 0; i != solver->num_effectors; ++i)
    {
        struct ik_node* tip = solver->effector_nodes[i];
        struct ik_effector* eff = tip->effector;
        union ik_vec3 tip_pos = tip->position;
        ikreal* target = solver->target_positions[i].f;

        /* Transform tip node position into same space as the effector target
         * position so weight can be applied. */
        while (tip != root)
        {
            ik_vec3_rotate_quat_conj(tip_pos.f, tip->rotation.f);
            tip = tip->parent; assert(tip != NULL);
            ik_vec3_add_vec3(tip_pos.f, tip->position.f);
        }

        /* lerp by effector weight to get weighted target position */
        ik_vec3_copy(target, eff->target_position.f);
        ik_vec3_sub_vec3(target, tip_pos.f);
        ik_vec3_mul_scalar(target, eff->weight);
        ik_vec3_add_vec3(target, tip_pos.f);

        /* TODO nlerp, need base node for that */
#if 0
static void
update(struct ik_node_data_t* tip, struct ik_node_data_t* base)
{
    struct ik_effector_t* effector =
        (struct ik_effector_t*)tip->attachment[IK_ATTACHMENT_EFFECTOR];

    /* lerp using effector weight to get actual target position */
    effector->actual_target = effector->target_position;
    ik_vec3_sub_vec3(effector->actual_target.f, tip->transform.t.position.f);
    ik_vec3_mul_scalar(effector->actual_target.f, effector->weight);
    ik_vec3_add_vec3(effector->actual_target.f, tip->transform.t.position.f);

    /* Fancy solver using nlerp, makes transitions look more natural */
    if (effector->features & IK_EFFECTOR_WEIGHT_NLERP && effector->weight < 1.0)
    {
        ikreal_t distance_to_target;
        union ik_vec3 base_to_effector;
        union ik_vec3 base_to_target;

        /* Need distance from base node to target and base to effector node */
        base_to_effector = tip->transform.t.position;
        base_to_target = effector->target_position;
        ik_vec3_sub_vec3(base_to_effector.f, base->transform.t.position.f);
        ik_vec3_sub_vec3(base_to_target.f, base->transform.t.position.f);

        /* The effective distance is a lerp between these two distances */
        distance_to_target = ik_vec3_length(base_to_target.f) * effector->weight;
        distance_to_target += ik_vec3_length(base_to_effector.f) * (1.0 - effector->weight);

        /* nlerp the target position by pinning it to the base node */
        ik_vec3_sub_vec3(effector->actual_target.f, base->transform.t.position.f);
        ik_vec3_normalize(effector->actual_target.f);
        ik_vec3_mul_scalar(effector->actual_target.f, distance_to_target);
        ik_vec3_add_vec3(effector->actual_target.f, base->transform.t.position.f);
    }
}
#endif
    }
}

/* ------------------------------------------------------------------------- */
static void
transform_target_to_local_space(ikreal target[3],
                                const struct ik_node* node,
                                const struct ik_node* root)
{
    const struct ik_node* parent = node->parent;
    if (parent != root)
        transform_target_to_local_space(target, parent, root);

    ik_vec3_sub_vec3(target, parent->position.f);
    ik_vec3_rotate_quat_conj(target, node->rotation.f);
}
static union ik_vec3
solve_chain_forwards_recurse(struct ik_chain* chain,
                             union ik_vec3** target_store,
                             const struct ik_node* root)
{
    union ik_vec3 target;
    struct ik_node* prev_segment_child;
    int avg_count;

    /* Target position for the tip of each chain is the average position of all
     * solved base node positions */
    avg_count = 0;
    ik_vec3_set_zero(target.f);
    CHAIN_FOR_EACH_CHILD(chain, child)
        union ik_vec3 base_pos = solve_chain_forwards_recurse(child, target_store, root);
        ik_vec3_add_vec3(target.f, base_pos.f);
        ++avg_count;
    CHAIN_END_EACH

    if (avg_count == 0)
    {
        target = *(*target_store)++;
        transform_target_to_local_space(target.f, chain_get_tip_node(chain), root);
    }
    else
        ik_vec3_div_scalar(target.f, avg_count);

    {
        ikreal dist;
        union ik_vec3 dir;
        union ik_quat delta;
        struct ik_node* child = chain_get_node(chain, 0);
        struct ik_node* parent = chain_get_node(chain, 1);

        /* Calculate target direction */
        dist = ik_vec3_length(target.f);
        dir = target;
        ik_vec3_div_scalar(dir.f, dist);

        /* Point segment to target */
        ik_quat_angle_of_nn(delta.f, dir.f);
        ik_quat_mul_quat(child->rotation.f, delta.f);
        CHAIN_FOR_EACH_CHILD(chain, child_chain)
            struct ik_node* first_child_node = chain_get_node(child_chain, chain_node_count(child_chain) - 2);
            ik_quat_mul_quat_conj(first_child_node->rotation.f, delta.f);
        CHAIN_END_EACH

        /* Calculate segment base node position if the tip were attached to
         * the target position, which becomes the next segment's target
         * position. NOTE: We assume the segment is pointing directly at the
         * target position here and therefore are alined with the Z axis in
         * local space. If in the future constraints are applied during forward
         * iteration, then this is no longer true. */
        ik_vec3_set(target.f, 0, 0, dist - child->position.v.z);

        /* Transform target into parent space */
        ik_vec3_rotate_quat(target.f, child->rotation.f);
        ik_vec3_add_vec3(target.f, parent->position.f);

        prev_segment_child = child;
    }

    CHAIN_FOR_EACH_SEGMENT_RANGE(chain, parent, child, 1, chain_segment_count(chain))
        ikreal dist;
        union ik_vec3 dir;
        union ik_quat delta;

        /* Calculate target direction */
        dist = ik_vec3_length(target.f);
        dir = target;
        ik_vec3_div_scalar(dir.f, dist);

        /* Point segment to target */
        ik_quat_angle_of_nn(delta.f, dir.f);
        ik_quat_mul_quat(child->rotation.f, delta.f);
        ik_quat_mul_quat_conj(prev_segment_child->rotation.f, delta.f);
        prev_segment_child = child;

        /* Calculate segment base node position if the tip were attached to
         * the target position, which becomes the next segment's target
         * position. NOTE: We assume the segment is pointing directly at the
         * target position here and therefore are alined with the Z axis in
         * local space. If in the future constraints are applied during forward
         * iteration, then this is no longer true. */
        ik_vec3_set(target.f, 0, 0, dist - child->position.v.z);

        /* Transform target into parent space */
        ik_vec3_rotate_quat(target.f, child->rotation.f);
        ik_vec3_add_vec3(target.f, parent->position.f);
    CHAIN_END_EACH

    return target;
}
static union ik_vec3
solve_chain_forwards(struct ik_solver_fabrik* solver)
{
    union ik_vec3* target_store = solver->target_positions;
    struct ik_node* root = chain_get_base_node(&solver->chain_tree);
    union ik_vec3 target = solve_chain_forwards_recurse(&solver->chain_tree, &target_store, root);

    /* This sets up the target position correctly for backwards iteration */
    ik_vec3_sub_vec3(target.f, root->position.f);
    ik_vec3_negate(target.f);
    ik_vec3_add_vec3(target.f, root->position.f);

    return target;
}

/* ------------------------------------------------------------------------- */
static void
solve_chain_backwards_constraints_recurse(struct ik_chain* chain, union ik_vec3 target)
{
    CHAIN_FOR_EACH_SEGMENT_R(chain, parent, child)
        ikreal dist;
        union ik_vec3 dir;
        union ik_quat delta;

        /* Transform target into this segment's space */
        ik_vec3_sub_vec3(target.f, parent->position.f);
        ik_vec3_rotate_quat_conj(target.f, child->rotation.f);

        /* Determine direction vector to child node position */
        dir = child->position;
        ik_vec3_sub_vec3(dir.f, target.f);
        dist = ik_vec3_length(dir.f);
        ik_vec3_div_scalar(dir.f, dist);

        /* Point segment at target */
        ik_quat_angle_of_nn(delta.f, dir.f);
        ik_quat_mul_quat(child->rotation.f, delta.f);

        if (child->constraint)
        {
            delta = child->rotation;
            child->constraint->apply(child->constraint, child->rotation.f);
            ik_quat_conj_rmul_quat(child->rotation.f, delta.f);
            ik_vec3_rotate_quat_conj(dir.f, delta.f);
        }

        ik_vec3_mul_scalar(dir.f, dist);
        ik_vec3_add_vec3(target.f, dir.f);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD(chain, child)
        solve_chain_backwards_constraints_recurse(child, target);
    CHAIN_END_EACH
}
static void
solve_chain_backwards_constraints(struct ik_solver_fabrik* solver, union ik_vec3 target)
{
    solve_chain_backwards_constraints_recurse(&solver->chain_tree, target);
}

/* ------------------------------------------------------------------------- */
static void
solve_chain_backwards_recurse(struct ik_chain* chain, union ik_vec3 target)
{
    CHAIN_FOR_EACH_SEGMENT_R(chain, parent, child)
        ikreal dist;
        union ik_vec3 dir;
        union ik_quat delta;

        /* Transform target into this segment's space */
        ik_vec3_sub_vec3(target.f, parent->position.f);
        ik_vec3_rotate_quat_conj(target.f, child->rotation.f);

        /* Determine direction vector to child node position */
        dir = child->position;
        ik_vec3_sub_vec3(dir.f, target.f);
        dist = ik_vec3_length(dir.f);
        ik_vec3_div_scalar(dir.f, dist);

        /* Point segment at target */
        ik_quat_angle_of_nn(delta.f, dir.f);
        ik_quat_mul_quat(child->rotation.f, delta.f);

        /* Calculate new target position */
        ik_vec3_mul_scalar(dir.f, child->position.v.z);
        ik_vec3_add_vec3(target.f, dir.f);
        ik_vec3_sub_vec3(target.f, child->position.f);
        ik_vec3_rotate_quat(target.f, delta.f);
        ik_vec3_add_vec3(target.f, child->position.f);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD(chain, child)
        solve_chain_backwards_recurse(child, target);
    CHAIN_END_EACH
}
static void
solve_chain_backwards(struct ik_solver_fabrik* solver, union ik_vec3 target)
{
    solve_chain_backwards_recurse(&solver->chain_tree, target);
}

/* ------------------------------------------------------------------------- */
/* TODO move into chain_tree.h */
static int
count_total_dead_nodes(const struct ik_chain* chain)
{
    int count = chain_dead_node_count(chain);
    CHAIN_FOR_EACH_CHILD(chain, child)
        count += count_total_dead_nodes(child);
    CHAIN_END_EACH
    return count;
}

/* ------------------------------------------------------------------------- */
static int
fabrik_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    int num_dead_nodes;
    int num_chains;
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    chain_tree_init(&solver->chain_tree);
    if (chain_tree_build(&solver->chain_tree, subtree) != 0)
        goto build_chain_tree_failed;

    solver->num_effectors = subtree_leaves(subtree);
    num_dead_nodes = count_total_dead_nodes(&solver->chain_tree);
    num_chains = count_chains(&solver->chain_tree);
    solver->num_intermediate_rotations = num_chains + num_dead_nodes;

    solver->intermediate_rotations = MALLOC(sizeof(*solver->intermediate_rotations) * solver->num_intermediate_rotations);
    if (solver->intermediate_rotations == NULL)
        goto alloc_effector_rotations_failed;

    solver->effector_nodes = MALLOC(sizeof(*solver->effector_nodes) * solver->num_effectors);
    if (solver->effector_nodes == NULL)
        goto alloc_effector_nodes_failed;

    solver->target_positions = MALLOC(sizeof(*solver->target_positions) * solver->num_effectors);
    if (solver->target_positions == NULL)
        goto alloc_target_positions_failed;

    store_effector_nodes(solver);
    validate_poles(solver);

    ik_log_printf(IK_DEBUG, "FABRIK: Initialized with %d end-effectors and %d dead nodes. %d chains were created.",
                  solver->num_effectors, num_dead_nodes, num_chains);

    return 0;

    alloc_target_positions_failed   : FREE(solver->effector_nodes);
    alloc_effector_nodes_failed     : FREE(solver->intermediate_rotations);
    alloc_effector_rotations_failed :
    build_chain_tree_failed         : chain_tree_deinit(&solver->chain_tree);
    return -1;
}

/* ------------------------------------------------------------------------- */
static void
fabrik_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    chain_tree_deinit(&solver->chain_tree);
    FREE(solver->target_positions);
    FREE(solver->effector_nodes);
    FREE(solver->intermediate_rotations);
}

/* ------------------------------------------------------------------------- */
static int
all_targets_reached(struct ik_solver_fabrik* solver, ikreal tol_squared)
{
    /* TODO broken
    int i;
    for (i = 0; i != solver->num_effectors; ++i)
    {
        struct ik_node* node = solver->effector_nodes[i];
        struct ik_effector* eff = node->effector;
        union ik_vec3 diff = node->position;

        ik_vec3_sub_vec3(diff.f, eff->target_position.f);
        if (ik_vec3_length_squared(diff.f) > tol_squared)
            return 0;
    }*/

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
fabrik_solve(struct ik_solver* solver_base)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;
    struct ik_algorithm* alg = solver->algorithm;
    int iteration = alg->max_iterations;
    ikreal tol_squared = alg->tolerance * alg->tolerance;

    ik_transform_chain_to_segmental_representation(&solver->chain_tree,
                                                   solver->intermediate_rotations,
                                                   solver->num_intermediate_rotations);
    update_target_data(solver);

    while (iteration-- > 0)
    {
        union ik_vec3 base_pos = solve_chain_forwards(solver);

        if (alg->features & IK_ALGORITHM_CONSTRAINTS)
            solve_chain_backwards_constraints(solver, base_pos);
        else
            solve_chain_backwards(solver, base_pos);

        if (all_targets_reached(solver, tol_squared))
            break;
    }

    ik_transform_chain_to_nodal_representation(&solver->chain_tree,
                                               solver->intermediate_rotations,
                                               solver->num_intermediate_rotations);

    return alg->max_iterations - iteration;
}

/* ------------------------------------------------------------------------- */
static void
fabrik_iterate_nodes_recursive(const struct ik_chain* chain, ik_solver_callback_func callback)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        fabrik_iterate_nodes_recursive(child, callback);
    CHAIN_END_EACH

    /* exclude base node */
    CHAIN_FOR_EACH_NODE_RANGE(chain, node, 0, chain_node_count(chain) - 1)
        callback(node);
    CHAIN_END_EACH
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
static void
fabrik_get_first_segment(const struct ik_solver* solver_base, struct ik_node** base, struct ik_node** tip)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    *base = chain_get_node(&solver->chain_tree, chain_node_count(&solver->chain_tree) - 1);
    *tip = chain_get_node(&solver->chain_tree, chain_node_count(&solver->chain_tree) - 2);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_FABRIK = {
    "fabrik",
    sizeof(struct ik_solver_fabrik),
    fabrik_init,
    fabrik_deinit,
    fabrik_solve,
    fabrik_iterate_nodes,
    fabrik_iterate_effector_nodes,
    fabrik_get_first_segment
};
