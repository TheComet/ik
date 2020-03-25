#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/quat.h"
#include "ik/solvers.h"
#include "ik/transform.h"
#include "ik/vec3.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

struct effector_data
{
    struct ik_effector* effector;
    union ik_vec3 lerped_position;
};

struct ik_solver
{
    IK_SOLVER_HEAD

    /* Used to store the initial transform for calculating joint rotations */
    union ik_transform* initial_transforms;

    /* More optimal data structure for FABRIK */
    struct ik_chain chain_tree;

    /*
     * Need to keep a list of all effectors (and their nodes) so it's easier to
     * check if targets have been reached. The lerped target positions are also
     * cached to make things more efficient.
     */
    struct vector_t effector_data_list;  /* list of effector_data* */
};

union position_direction
{
    struct {
        union ik_vec3 pos;
        union ik_vec3 dir;
    } v;
    ikreal f[6];
};

/* ------------------------------------------------------------------------- */
static union position_direction
solve_chain_forwards_with_target_rotation(struct ik_chain* chain)
{
    int node_count, node_idx;
    int average_count;
    union position_direction target;

    /*
     * Target position (and direction) is the average of all solved child chain base positions.
     */
    ik_vec3_set_zero(target.v.pos.f);
    ik_vec3_set_zero(target.v.dir.f);
    average_count = 0;
    CHAIN_FOR_EACH_CHILD(chain, child)
        union position_direction child_posdir = solve_chain_forwards_with_target_rotation(child);
        ik_vec3_add_vec3(target.v.pos.f, child_posdir.v.pos.f);
        ik_vec3_add_vec3(target.v.dir.f, child_posdir.v.dir.f);
        ++average_count;
    CHAIN_END_EACH

    /*
     * If there are no child chains, then the first node in the chain must
     * contain an effector. The target position is the effector's target
     * position.
     *
     * If we are an intermediate chain, then our target position is not the
     * effector's target position (there is no effector), instead, it is the
     * average position of all of child chains.
     */
    if (average_count == 0)
    {
        struct ik_node* effector_node = chain_get_node(chain, 0);
        struct ik_effector* effector = effector_node->effector;
        target.v.pos = effector->actual_target;

        /* TODO This "global direction" could be made configurable if needed */
        target.v.dir.v.x = 0.0;
        target.v.dir.v.y = 0.0;
        target.v.dir.v.z = 1.0;
        ik_vec3_rotate_quat(target.v.dir.f, effector->target_rotation.f);
    }
    else
    {
        ik_vec3_div_scalar(target.v.pos.f, average_count);
        ik_vec3_normalize(target.v.dir.f);
    }

    /*
     * Iterate through each segment and apply the FABRIK solver.
     */
    node_count = chain_length(chain);
    for (node_idx = 0; node_idx < node_count - 1; ++node_idx)
    {
        struct ik_node* child_node  = chain_get_node(chain, node_idx + 0);
        struct ik_node* parent_node = chain_get_node(chain, node_idx + 1);

        /* move node to target */
        child_node->position = target.v.pos;

        /* lerp between direction vector and segment vector */
        ik_vec3_sub_vec3(target.v.pos.f, parent_node->position.f);        /* segment vector */
        ik_vec3_normalize(target.v.pos.f);                                /* normalize so we have segment direction vector */
        ik_vec3_sub_vec3(target.v.pos.f, target.v.dir.f);             /* for lerp, subtract target direction... */
        ik_vec3_mul_scalar(target.v.pos.f, parent_node->rotation_weight); /* ...mul with weight... */
        ik_vec3_add_vec3(target.v.pos.f, parent_node->position.f);        /* ...and attach this lerp'd direction to the parent node */

        /* point segment to previous node */
        ik_vec3_sub_vec3(target.v.pos.f, child_node->position.f);         /* this computes the correct direction the segment should have */
        ik_vec3_normalize(target.v.pos.f);
        ik_vec3_mul_scalar(target.v.pos.f, child_node->dist_to_parent);
        ik_vec3_add_vec3(target.v.pos.f, child_node->position.f);         /* attach to child -- this is the new target for the next segment */
    }

    return target;
}

/* ------------------------------------------------------------------------- */
static union ik_vec3
solve_chain_forwards(struct ik_chain* chain)
{
    int node_count, node_idx;
    int average_count;
    union ik_vec3 target_position;

    /*
     * Target position is the average of all solved child chain base positions.
     */
    ik_vec3_set_zero(target_position.f);
    average_count = 0;
    CHAIN_FOR_EACH_CHILD(chain, child)
        union ik_vec3 child_base_position = solve_chain_forwards(child);
        ik_vec3_add_vec3(target_position.f, child_base_position.f);
        ++average_count;
    CHAIN_END_EACH

    /*
     * If there are no child chains, then the first node in the chain must
     * contain an effector. The target position is the effector's target
     * position. Otherwise, average the data we've been accumulating from the
     * child chains.
     */
    if (average_count == 0)
    {
        struct ik_node* effector_node = chain_get_node(chain, 0);
        struct ik_effector* effector = effector_node->effector;
        target_position = effector->actual_target;
    }
    else
    {
        ik_vec3_div_scalar(target_position.f, average_count);
    }

    /*
     * Iterate through each segment and apply the FABRIK solver.
     */
    node_count = chain_length(chain);
    for (node_idx = 0; node_idx < node_count - 1; ++node_idx)
    {
        struct ik_node* child_node  = chain_get_node(chain, node_idx + 0);
        struct ik_node* parent_node = chain_get_node(chain, node_idx + 1);

        /* move node to target */
        child_node->position = target_position;

        /* point segment to previous node and set target position to its end */
        ik_vec3_sub_vec3(target_position.f, parent_node->position.f);        /* parent points to child */
        ik_vec3_normalize(target_position.f);                                /* normalise */
        ik_vec3_mul_scalar(target_position.f, -child_node->dist_to_parent);  /* child points to parent */
        ik_vec3_add_vec3(target_position.f, child_node->position.f);         /* attach to child -- this is the new target for next iteration */
    }

    return target_position;
}

/* ------------------------------------------------------------------------- */
static void
solve_chain_backwards(struct ik_chain* chain, union ik_vec3 target_position)
{
    int node_idx = chain_length(chain) - 1;

    /*
     * The base node must be set to the target position before iterating.
     * XXX: Why is this conditional?
     */
    if (node_idx > 1)
    {
        struct ik_node* base_node = chain_get_node(chain, node_idx);
        base_node->position = target_position;
    }

    /*
     * Iterate through each segment the other way around and apply the FABRIK
     * solver.
     */
    while (node_idx-- > 0)
    {
        struct ik_node* child_node  = chain_get_node(chain, node_idx + 0);
        struct ik_node* parent_node = chain_get_node(chain, node_idx + 1);

        /* point segment to child node and set target position to its beginning */
        ik_vec3_sub_vec3(target_position.f, child_node->position.f); /* child points to parent */
        ik_vec3_normalize(target_position.f);                                  /* normalise */
        ik_vec3_mul_scalar(target_position.f, -child_node->dist_to_parent);    /* parent points to child */
        ik_vec3_add_vec3(target_position.f, parent_node->position.f);/* attach to parent -- this is the new target */

        /* move node to target */
        child_node->position = target_position;
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        solve_chain_backwards(child, target_position);
    CHAIN_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
calculate_joint_rotations_for_chain(struct ik_chain* chain);
static void
recurse_into_children(struct ik_chain* chain)
{
    int average_count = 0;
    struct ik_quat average_rotation;
    ik_quat_set(average_rotation.f, 0, 0, 0, 0);

    /* Recurse into children chains */
    CHAIN_FOR_EACH_CHILD(chain, child)
        struct ik_quat_t rotation;
        calculate_joint_rotations_for_chain(child);

        rotation = chain_get_base_node(chain)->rotation;

        /*
         * Averaging quaternions taken from here
         * http://wiki.unity3d.com/index.php/Averaging_Quaternions_and_Vectors
         */
        ik_quat_ensure_positive_sign(rotation.f);
        ik_quat_add_quat(average_rotation.f, rotation.f);
        ++average_count;
    CHAIN_END_EACH

    /*
     * Assuming there was more than 1 child chain and assuming we aren't the
     * base node, then the child chains we just iterated must share the same
     * sub-base node (which is our tip node). Average the accumulated
     * quaternion and set this node's correct solved rotation.
     */
    if (average_count > 0)
    {
        ik_quat_div_scalar(average_rotation.f, average_count);
        ik_quat_normalize(average_rotation.f);
        chain_get_tip_node(chain)->rotation = average_rotation;
    }
}

/* ------------------------------------------------------------------------- */
static void
calculate_delta_rotation_of_each_segment(struct ik_chain* chain)
{
    /*
     * Calculate all of the delta rotations of the joints and store them into
     * node->rotation.
     */
    int node_idx = chain_length(chain) - 1;
    while (node_idx-- > 0)
    {
        struct ik_node* child_node  = chain_get_node(chain, node_idx + 0);
        struct ik_node* parent_node = chain_get_node(chain, node_idx + 1);

        /* calculate vectors for original and solved segments */
        union ik_vec3 segment_original = child_node->FABRIK.initial_position;
        union ik_vec3 segment_solved   = child_node->position;
        ik_vec3_sub_vec3(segment_original.f, parent_node->FABRIK.initial_position.f);
        ik_vec3_sub_vec3(segment_solved.f, parent_node->position.f);
        ik_quat_angle(parent_node->rotation.f, segment_solved.f, segment_original.f);
    }
}

/* ------------------------------------------------------------------------- */
static void
calculate_joint_rotations_for_chain(struct ik_chain* chain)
{
    struct ik_node* effector_node;

    assert(chain_length(chain) >= 2);

    /*
     * Calculates the "global" (world) angles of each joint and writes them to
     * each node->rotation field.
     *
     * The angle between the original and solved segments are calculated using
     * standard vector math (dot product). The axis of rotation is calculated
     * with the cross product. From this data, a quaternion is inited,
     * describing this delta rotation. Finally, in order to make the rotations
     * global instead of relative, the delta rotation is multiplied with
     * node->original_rotation, which should be a quaternion describing the
     * node's global rotation in the unsolved tree.
     *
     * The rotation of the base joint in the chain is returned so it can be
     * averaged by parent chains.
     */
    recurse_into_children(chain);
    calculate_delta_rotation_of_each_segment(chain);

    /*
     * It's not possible to calculate rotations for the end effector nodes,
     * since there are no child nodes to compare. Because we're in global space
     * leaving the rotations untouched will cause the end effector to keep a
     * global orientation regardless of what the rest of the tree is doing.
     * this may or may not be desirable, so the user can enable the effector
     * node to inherit its parent rotation.
     */
    effector_node = chain_get_tip_node(chain);
    if (effector_node->effector && !(effector_node->effector->flags & IK_EFFECTOR_KEEP_ORIENTATION))
    {
        effector_node->rotation = chain_get_node(chain, 1)->rotation; /* parent node */
    }

    /*
     * Finally, apply initial global rotations to calculated delta rotations to
     * obtain the solved global rotations.
     */
    CHAIN_FOR_EACH_NODE(chain, node_base)
        struct ik_node* node = node_base;
        ik_quat_mul_quat(node->rotation.f, node->FABRIK.initial_rotation.f);
    CHAIN_END_EACH
}
static void
calculate_joint_rotations(struct vector_t* chain_list)
{
    VECTOR_FOR_EACH(chain_list, struct ik_chain, chain)
        calculate_joint_rotations_for_chain(chain);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
store_initial_transform_for_chain(struct ik_chain* chain)
{
    CHAIN_FOR_EACH_NODE(chain, node_base)
        struct ik_node* node = node_base;
        node->FABRIK.initial_position = node->position;
        node->FABRIK.initial_rotation = node->rotation;
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD(chain, child)
        store_initial_transform_for_chain(child);
    CHAIN_END_EACH
}
static void
store_initial_transform(const struct vector_t* chain_list)
{
    VECTOR_FOR_EACH(chain_list, struct ik_chain, chain)
        store_initial_transform_for_chain(chain);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static int
FABRIK_init(struct ik_solver* solver)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
static void
FABRIK_deinit(struct ik_solver* solver)
{
}

/* ------------------------------------------------------------------------- */
static int
FABRIK_rebuild(struct ik_solver* solver, const struct ik_subtree* subtree)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
static void
FABRIK_update_translations(struct ik_solver* solver)
{

}

/* ------------------------------------------------------------------------- */
static int
FABRIK_solve(struct ik_solver* solver)
{
    struct ik_algorithm* alg = solver->algorithm;
    int iteration = alg->max_iterations;
    ikreal tolerance_squared = alg->tolerance * alg->tolerance;

    /* Tree is in local space -- FABRIK needs only global node positions */
    ik_transform_chain_list(solver, IK_TRANSFORM_L2G);

    /*
     * Joint rotations are calculated by comparing positional differences
     * before and after solving the tree. This comparison needs to occur in
     * global space (doesn't work in local as far as I can see). Store the
     * positions and locations before solving for later.
     */
    if (alg->features & IK_ALGORITHM_JOINT_ROTATIONS)
        store_initial_transform(&solver->chain_list);

    while (iteration-- > 0)
    {
        /* Check if all effectors are within range */
        ALGORITHM_FOR_EACH_EFFECTOR_NODE(solver, node)
            union ik_vec3 diff = node->position;
            ik_vec3_sub_vec3(diff.f, node->effector->target_position.f);
            if (ik_vec3_length_squared(diff.f) > tolerance_squared)
            {
                goto hasnt_converged;
            }
        ALGORITHM_END_EACH
        result = IK_RESULT_CONVERGED;
        break;
        hasnt_converged:

        ALGORITHM_FOR_EACH_CHAIN(solver, chain)
            struct  ik_node* base_node;
            int idx;

            /*
             * The solver assumes chains have at least one bone. This should
             * be asserted while building the chain trees, but it can't hurt
             * to double check
             */
            idx = chain_length(chain) - 1;
            assert(idx > 0);

            base_node = chain_get_node(chain, idx);

            if (solver->features & IK_ALGORITHM_TARGET_ROTATIONS)
                solve_chain_forwards_with_target_rotation(chain);
            else
                solve_chain_forwards(chain);

            if (solver->features & IK_ALGORITHM_CONSTRAINTS)
                solve_chain_backwards_with_constraints(chain, base_node->position, base_node->rotation, base_node->position);
            else
                solve_chain_backwards(chain, base_node->position);
        ALGORITHM_END_EACH

    }

    if (solver->features & IK_ALGORITHM_JOINT_ROTATIONS)
        calculate_joint_rotations(&solver->chain_list);

    /* Transform back to local space now that solving is complete */
    ik_transform_chain_list(solver, IK_TRANSFORM_G2L);

    return result;
}

/* ------------------------------------------------------------------------- */
static void
FABRIK_iterate_nodes(const struct ik_solver* solver, ik_solver_callback_func cb)
{

}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_FABRIK = {
    "FABRIK",
    sizeof(struct ik_solver),
    FABRIK_init,
    FABRIK_deinit,
    FABRIK_rebuild,
    FABRIK_update_translations,
    FABRIK_solve,
    FABRIK_iterate_nodes
};
