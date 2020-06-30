#include "ik/chain_tree.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/solver.h"
#include "ik/subtree.h"

#include "cstructures/memory.h"

#include <stddef.h>
#include <math.h>

struct ik_solver_fabrik
{
    IK_SOLVER_HEAD

    struct ik_chain chain_tree;
    union ik_quat* effector_rotations;
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
    CHAIN_FOR_EACH_NODE_RANGE(chain, node, 0, chain_length(chain) - 1)
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
validate_poles(const struct ik_solver_fabrik* solver, const struct ik_chain* chain)
{
    if (validate_poles_recursive(solver, chain))
    {
        ik_log_printf(IK_WARN, "(\"%s\" algorithm): Poles only make sense when attached to the end of chains, such as effector nodes, or nodes with multiple children.", solver->impl.name);
    }
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
    int idx;

    CHAIN_FOR_EACH_CHILD(chain, child)
        convert_rotations_to_segments_recursive(child, chain_child_count(chain));
    CHAIN_END_EACH

    /* Copy all rotations from parent to child node, excluding base node. Base
     * node requires special attention (see below) */
    for (idx = 0; idx < (int)chain_length(chain) - 2; ++idx)
    {
        struct ik_node* child  = chain_get_node(chain, idx + 0);
        struct ik_node* parent = chain_get_node(chain, idx + 1);

        ik_quat_copy(child->rotation.f, parent->rotation.f);
    }

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
        struct ik_node* child  = chain_get_node(chain, chain_length(chain) - 2);
        struct ik_node* parent = chain_get_node(chain, chain_length(chain) - 1);

        ik_quat_angle_of(child->rotation.f, child->position.f);
        ik_quat_rmul_quat(child->rotation.f, parent->rotation.f);
        ik_vec3_set(child->position.f, 0, 0, ik_vec3_length(child->position.f));
    }
    else
    {
        struct ik_node* child  = chain_get_node(chain, chain_length(chain) - 2);
        struct ik_node* parent = chain_get_node(chain, chain_length(chain) - 1);

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
    int idx;  /* must be signed, because chain_length(chain) - 3 can be negative */

    /* Copy all rotations from child back to parent node excluding the base node
     * and tip node */
    for (idx = (int)chain_length(chain) - 3; idx >= 0; idx--)
    {
        struct ik_node* parent = chain_get_node(chain, idx + 1);
        struct ik_node* child  = chain_get_node(chain, idx + 0);

        ik_quat_copy(parent->rotation.f, child->rotation.f);
    }

    if (chain_child_count(chain) > 1)
    {
        struct ik_node* tip_node = chain_get_tip_node(chain);
        ikreal* tip_rot = tip_node->rotation.f;

        /* restore base node rotation by averaging all child rotations */
        ik_quat_set(tip_rot, 0, 0, 0, 0);
        CHAIN_FOR_EACH_CHILD(chain, child)
            struct ik_node* first_child_node = chain_get_node(child, chain_length(child) - 2);
            ik_quat_ensure_positive_sign(first_child_node->rotation.f);
            ik_quat_add_quat(tip_rot, first_child_node->rotation.f);
        CHAIN_END_EACH

        ik_quat_div_scalar(tip_rot, chain_child_count(chain));
        ik_quat_normalize(tip_rot);

        /* restore translations */
        ik_quat_conj(tip_rot);
        CHAIN_FOR_EACH_CHILD(chain, child)
            struct ik_node* first_child_node = chain_get_node(child, chain_length(child) - 2);
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
    base = chain_get_node(&solver->chain_tree, chain_length(&solver->chain_tree) - 1);
    child = chain_get_node(&solver->chain_tree, chain_length(&solver->chain_tree) - 1);
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
static int
fabrik_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    solver->effector_rotations = MALLOC(sizeof(*solver->effector_rotations) * subtree_leaves(subtree));
    if (solver->effector_rotations == NULL)
        goto alloc_effector_rotations_failed;

    if (chain_tree_init(&solver->chain_tree) != 0)
        goto chain_tree_init_failed;
    if (chain_tree_build(&solver->chain_tree, subtree) != 0)
        goto build_chain_tree_failed;

    validate_poles(solver, &solver->chain_tree);

    return 0;

    build_chain_tree_failed         : chain_tree_deinit(&solver->chain_tree);
    chain_tree_init_failed          : FREE(solver->effector_rotations);
    alloc_effector_rotations_failed : return -1;
}

/* ------------------------------------------------------------------------- */
static void
fabrik_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    chain_tree_deinit(&solver->chain_tree);
    FREE(solver->effector_rotations);
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
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    convert_rotations_to_segments(solver);
    convert_rotations_to_nodes(solver);
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
