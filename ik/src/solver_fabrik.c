#include "ik/chain_tree.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/solver.h"
#include "ik/subtree.h"
#include "ik/transform.h"

#include "cstructures/memory.h"

#include <stddef.h>
#include <math.h>

struct ik_solver_fabrik
{
    IK_SOLVER_HEAD

    struct ik_chain chain_tree;
    struct ik_node* effector_nodes;
    union ik_quat* effector_rotations;
    union ik_vec3* target_positions;

    int num_effectors;
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
store_effector_nodes_recursive(struct ik_node** effector_nodes_store, const struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        store_effector_nodes_recursive(effector_nodes_store, child);
    CHAIN_END_EACH

    if (chain_child_count(chain) == 0)
    {
        *effector_nodes_store = chain_get_tip_node(chain);
        (*effector_nodes_store)++;
    }
}
static void
store_effector_nodes(struct ik_solver_fabrik* solver)
{
    struct ik_node* effector_nodes_store = solver->effector_nodes;
    store_effector_nodes_recursive(&effector_nodes_store, &solver->chain_tree);

    /* sanity check */
    assert(effector_nodes_store - solver->effector_nodes == solver->num_effectors);
}

/* ------------------------------------------------------------------------- */
static void
store_effector_node_rotations(union ik_quat** rotations_store, const struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        store_effector_node_rotations(rotations_store, child);
    CHAIN_END_EACH

    if (chain_child_count(chain) == 0)
    {
        ik_quat_copy((*rotations_store)->f, chain_get_tip_node(chain)->rotation.f);
        (*rotations_store)++;
    }
}

/* ------------------------------------------------------------------------- */
static void
restore_effector_node_rotations(union ik_quat** rotations_store, const struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        restore_effector_node_rotations(rotations_store, child);
    CHAIN_END_EACH

    if (chain_child_count(chain) == 0)
    {
        ik_quat_copy(chain_get_tip_node(chain)->rotation.f, (*rotations_store)->f);
        (*rotations_store)++;
    }
}

/* ------------------------------------------------------------------------- */
static void
convert_rotations_to_segments_recursive(const struct ik_chain* chain,
                                        unsigned int sibling_count)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        convert_rotations_to_segments_recursive(child, chain_child_count(chain));
    CHAIN_END_EACH

    /* Copy all rotations from parent to child node, excluding base node. Base
     * node requires special attention (see below) */
    CHAIN_FOR_EACH_SEGMENT_RANGE(chain, parent, child, 0, chain_segment_count(chain) - 1)
        ik_quat_copy(child->rotation.f, parent->rotation.f);
    CHAIN_END_EACH

    /*
     * Nodes that have siblings will have a translation that is not [0, 0, 1],
     * and their parent node will have a rotation that is an average of all
     * sibling rotations.
     *
     * Need to calculate the parent node rotation that would place the child
     * node at [0, 0, 1] and store that rotation in the child node.
     */
    if (sibling_count > 1)
    {
        struct ik_node* child  = chain_get_node(chain, chain_node_count(chain) - 2);
        struct ik_node* parent = chain_get_node(chain, chain_node_count(chain) - 1);

        ik_quat_angle_of(child->rotation.f, child->position.f);
        ik_quat_rmul_quat(child->rotation.f, parent->rotation.f);
        ik_vec3_set(child->position.f, 0, 0, ik_vec3_length(child->position.f));
    }
    else
    {
        struct ik_node* child  = chain_get_node(chain, chain_node_count(chain) - 2);
        struct ik_node* parent = chain_get_node(chain, chain_node_count(chain) - 1);

        ik_quat_copy(child->rotation.f, parent->rotation.f);
    }
}
static void
convert_rotations_to_segments(struct ik_solver_fabrik* solver)
{
    union ik_quat* rotations_store;

    /*
     * Copy all effector node rotations into a pre-allocated array stored in the
     * solver object so we can restore them later. They will be overwritten
     * by the parent node rotation in the next section of code.
     */
    rotations_store = solver->effector_rotations;
    store_effector_node_rotations(&rotations_store, &solver->chain_tree);

    /* Do conversion */
    convert_rotations_to_segments_recursive(&solver->chain_tree, 1);
}

/* ------------------------------------------------------------------------- */
static void
convert_rotations_to_nodes_recursive(const struct ik_chain* chain)
{
    /* Copy all rotations from child back to parent node excluding the base node
     * and tip node */
    CHAIN_FOR_EACH_SEGMENT_RANGE_R(chain, parent, child, 0, chain_segment_count(chain) - 1)
        ik_quat_copy(parent->rotation.f, child->rotation.f);
    CHAIN_END_EACH

    if (chain_child_count(chain) > 1)
    {
        struct ik_node* tip_node = chain_get_tip_node(chain);
        ikreal* tip_rot = tip_node->rotation.f;

        /* restore base node rotation by averaging all child rotations */
        ik_quat_set(tip_rot, 0, 0, 0, 0);
        CHAIN_FOR_EACH_CHILD(chain, child)
            struct ik_node* first_child_node = chain_get_node(child, chain_node_count(child) - 2);
            ik_quat_ensure_positive_sign(first_child_node->rotation.f);
            ik_quat_add_quat(tip_rot, first_child_node->rotation.f);
        CHAIN_END_EACH

        ik_quat_div_scalar(tip_rot, chain_child_count(chain));
        ik_quat_normalize(tip_rot);

        /* restore translations */
        ik_quat_conj(tip_rot);
        CHAIN_FOR_EACH_CHILD(chain, child)
            struct ik_node* first_child_node = chain_get_node(child, chain_node_count(child) - 2);
            ik_quat_rmul_quat(first_child_node->rotation.f, tip_rot);
            ik_vec3_rotate_quat(first_child_node->position.f, first_child_node->rotation.f);
        CHAIN_END_EACH
        ik_quat_conj(tip_rot);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        convert_rotations_to_nodes_recursive(child);
    CHAIN_END_EACH
}
static void
convert_rotations_to_nodes(struct ik_solver_fabrik* solver)
{
    union ik_quat* rotations_store;
    struct ik_node* base;
    struct ik_node* child;

    /*
     * The recursive function copies everything except for the base node. Copy
     * base node rotation here.
     */
    base = chain_get_node(&solver->chain_tree, chain_node_count(&solver->chain_tree) - 1);
    child = chain_get_node(&solver->chain_tree, chain_node_count(&solver->chain_tree) - 1);
    ik_quat_copy(base->rotation.f, child->rotation.f);

    /* Do conversion */
    convert_rotations_to_nodes_recursive(&solver->chain_tree);

    /*
     * Copy original effector node rotations back from pre-allocated buffer into
     * the tree.
     */
    rotations_store = solver->effector_rotations;
    restore_effector_node_rotations(&rotations_store, &solver->chain_tree);
}

/* ------------------------------------------------------------------------- */
static void
update_target_data(struct ik_solver_fabrik* solver)
{

}

/* ------------------------------------------------------------------------- */
static union ik_vec3
solve_chain_forwards_recurse(struct ik_chain* chain, union ik_vec3** target_store)
{
    union ik_vec3 target;
    int avg_count;

    /* Target position for the tip of each chain is the average position of all
     * solved base node positions */
    avg_count = 0;
    ik_vec3_set_zero(target.f);
    CHAIN_FOR_EACH_CHILD(chain, child)
        union ik_vec3 base_pos = solve_chain_forwards_recurse(child, target_store);
        ik_vec3_add_vec3(target.f, base_pos.f);
        ++avg_count;
    CHAIN_END_EACH

    if (avg_count == 0)
        target = *(*target_store)++;
    else
        ik_vec3_div_scalar(target.f, avg_count);

    CHAIN_FOR_EACH_SEGMENT(chain, parent, child)
        ikreal tdist;
        union ik_vec3 tdir;
        union ik_quat delta;

        /* Calculate target direction */
        tdist = ik_vec3_length(target.f);
        tdir = target;
        ik_vec3_div_scalar(tdir.f, tdist);

        /* Point segment to target */
        ik_quat_angle_of_no_normalize(delta.f, tdir.f);
        ik_quat_mul_quat(child->rotation.f, delta.f);

        /* Calculate segment base node position if the tip were attached to
         * the target position, which becomes the next segment's target
         * position. */
        ik_vec3_set(target.f, 0, 0, tdist - child->position.v.z);

        /* Transform target into parent space */
        ik_vec3_rotate_quat_conj(target.f, child->rotation.f);
        ik_vec3_add_vec3(target.f, parent->position.f);
    CHAIN_END_EACH

    return target;
}
static union ik_vec3
solve_chain_forwards(struct ik_solver_fabrik* solver)
{
    union ik_vec3* target_store = solver->target_positions;
    return solve_chain_forwards_recurse(&solver->chain_tree, &target_store);
}

/* ------------------------------------------------------------------------- */
static void
solve_chain_backwards_recurse(struct ik_chain* chain, union ik_vec3 target)
{
    CHAIN_FOR_EACH_SEGMENT_R(chain, parent, child)
        union ik_vec3 dir;
        union ik_quat delta;
        ik_vec3_sub_vec3(target.f, parent->position.f);
        ik_vec3_rotate_quat(target.f, child->rotation.f);

        dir = child->position;
        ik_vec3_sub_vec3(dir.f, target.f);
        ik_quat_angle_of(delta.f, dir.f);
    CHAIN_END_EACH
}
static void
solve_chain_backwards(struct ik_solver_fabrik* solver, union ik_vec3 target)
{
    solve_chain_backwards_recurse(&solver->chain_tree, target);
}

/* ------------------------------------------------------------------------- */
static int
fabrik_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    solver->num_effectors = subtree_leaves(subtree);

    solver->effector_rotations = MALLOC(sizeof(*solver->effector_rotations) * solver->num_effectors);
    if (solver->effector_rotations == NULL)
        goto alloc_effector_rotations_failed;

    solver->effector_nodes = MALLOC(sizeof(*solver->effector_nodes) * solver->num_effectors);
    if (solver->effector_nodes == NULL)
        goto alloc_effector_nodes_failed;

    solver->target_positions = MALLOC(sizeof(*solver->target_positions) * solver->num_effectors);
    if (solver->target_positions == NULL)
        goto alloc_target_positions_failed;

    if (chain_tree_init(&solver->chain_tree) != 0)
        goto chain_tree_init_failed;
    if (chain_tree_build(&solver->chain_tree, subtree) != 0)
        goto build_chain_tree_failed;

    store_effector_nodes(solver);
    validate_poles(solver);

    return 0;

    build_chain_tree_failed         : chain_tree_deinit(&solver->chain_tree);
    chain_tree_init_failed          : FREE(solver->target_positions);
    alloc_target_positions_failed   : FREE(solver->effector_nodes);
    alloc_effector_nodes_failed     : FREE(solver->effector_rotations);
    alloc_effector_rotations_failed : return -1;
}

/* ------------------------------------------------------------------------- */
static void
fabrik_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    chain_tree_deinit(&solver->chain_tree);
    FREE(solver->target_positions);
    FREE(solver->effector_nodes);
    FREE(solver->effector_rotations);
}

/* ------------------------------------------------------------------------- */
static int
all_targets_reached(struct ik_solver_fabrik* solver, ikreal tol_squared)
{
    int i;
    for (i = 0; i != solver->num_effectors; ++i)
    {
        struct ik_node* node = &solver->effector_nodes[i];
        struct ik_effector* eff = node->effector;
        union ik_vec3 diff = node->position;

        ik_vec3_sub_vec3(diff.f, eff->target_position.f);
        if (ik_vec3_length_squared(diff.f) <= tol_squared)
            return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
static void
fabrik_solve(struct ik_solver* solver_base)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;
    struct ik_algorithm* alg = solver->algorithm;
    int iteration = alg->max_iterations;
    ikreal tol_squared = alg->tolerance * alg->tolerance;

    convert_rotations_to_segments(solver);
    update_target_data(solver);

    while (iteration-- > 0)
    {
        if (all_targets_reached(solver, tol_squared))
            break;

        solve_chain_backwards(solver, solve_chain_forwards(solver));
    }

    convert_rotations_to_nodes(solver);
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
struct ik_solver_interface ik_solver_FABRIK = {
    "fabrik",
    sizeof(struct ik_solver_fabrik),
    fabrik_init,
    fabrik_deinit,
    fabrik_solve,
    fabrik_iterate_nodes,
    fabrik_iterate_effector_nodes
};
