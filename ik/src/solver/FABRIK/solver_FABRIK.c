#include "ik/solver_FABRIK.h"
#include "ik/chain.h"
#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/node.h"
#include "ik/bstv.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/transform.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

struct position_direction_t
{
    union {
        struct {
            vec3_t position;
            vec3_t direction;
        };
        ikreal_t f[6];
    };
};

/* ------------------------------------------------------------------------- */
uintptr_t
ik_solver_FABRIK_type_size(void)
{
    return sizeof(struct ik_solver_t);
}

/* ------------------------------------------------------------------------- */
static struct position_direction_t
solve_chain_forwards_with_target_rotation(struct chain_t* chain)
{
    int node_count, node_idx;
    int average_count;
    struct position_direction_t target;

    /*
     * Target position (and direction) is the average of all solved child chain base positions.
     */
    vec3_set_zero(target.position.f);
    vec3_set_zero(target.direction.f);
    average_count = 0;
    CHAIN_FOR_EACH_CHILD(chain, child)
        struct position_direction_t child_posdir = solve_chain_forwards_with_target_rotation(child);
        vec3_add_vec3(target.position.f, child_posdir.position.f);
        vec3_add_vec3(target.direction.f, child_posdir.direction.f);
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
        struct ik_node_t* effector_node = chain_get_node(chain, 0);
        struct ik_effector_t* effector = effector_node->effector;
        target.position = effector->_actual_target;

        /* TODO This "global direction" could be made configurable if needed */
        target.direction.x = 0.0;
        target.direction.y = 0.0;
        target.direction.z = 1.0;
        quat_rotate_vec(target.direction.f, effector->target_rotation.f);
    }
    else
    {
        vec3_div_scalar(target.position.f, average_count);
        vec3_normalize(target.direction.f);
    }

    /*
     * Iterate through each segment and apply the FABRIK algorithm.
     */
    node_count = chain_length(chain);
    for (node_idx = 0; node_idx < node_count - 1; ++node_idx)
    {
        struct ik_node_t* child_node  = chain_get_node(chain, node_idx + 0);
        struct ik_node_t* parent_node = chain_get_node(chain, node_idx + 1);

        /* move node to target */
        child_node->position = target.position;

        /* lerp between direction vector and segment vector */
        vec3_sub_vec3(target.position.f, parent_node->position.f);        /* segment vector */
        vec3_normalize(target.position.f);                                /* normalize so we have segment direction vector */
        vec3_sub_vec3(target.position.f, target.direction.f);             /* for lerp, subtract target direction... */
        vec3_mul_scalar(target.position.f, parent_node->rotation_weight); /* ...mul with weight... */
        vec3_add_vec3(target.position.f, parent_node->position.f);        /* ...and attach this lerp'd direction to the parent node */

        /* point segment to previous node */
        vec3_sub_vec3(target.position.f, child_node->position.f);         /* this computes the correct direction the segment should have */
        vec3_normalize(target.position.f);
        vec3_mul_scalar(target.position.f, child_node->dist_to_parent);
        vec3_add_vec3(target.position.f, child_node->position.f);         /* attach to child -- this is the new target for the next segment */
    }

    return target;
}

/* ------------------------------------------------------------------------- */
vec3_t
solve_chain_forwards_with_constraints(struct chain_t* chain)
{
    int node_count, node_idx;
    int average_count;
    vec3_t target_position;

    /*
     * Target position is the average of all solved child chain base positions.
     */
    vec3_set_zero(target_position.f);
    average_count = 0;
    CHAIN_FOR_EACH_CHILD(chain, child)
        vec3_t child_base_position = solve_chain_forwards_with_constraints(child);
        vec3_add_vec3(target_position.f, child_base_position.f);
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
        struct ik_node_t* effector_node = chain_get_node(chain, 0);
        struct ik_effector_t* effector = effector_node->effector;
        target_position = effector->_actual_target;
    }
    else
    {
        vec3_div_scalar(target_position.f, average_count);
    }

    /*
     * Iterate through each segment and apply the FABRIK algorithm.
     */
    node_count = chain_length(chain);
    for (node_idx = 0; node_idx < node_count - 1; ++node_idx)
    {
        vec3_t initial_segment;
        struct ik_node_t* child_node  = chain_get_node(chain, node_idx + 0);
        struct ik_node_t* parent_node = chain_get_node(chain, node_idx + 1);

        /*
         * Need the initial (unsolved) segment so we can calculate joint
         * rotation for constraints.
         */
        vec3_copy(initial_segment.f, child_node->initial_position.f);
        vec3_sub_vec3(initial_segment.f, parent_node->initial_position.f);
        vec3_normalize(initial_segment.f);

        /* move node to target */
        child_node->position = target_position;

        /*
         * target_position will be a directional vector pointing from the new
         * target position to the previous node,
         */
        vec3_sub_vec3(target_position.f, parent_node->position.f);        /* parent points to child */
        vec3_normalize(target_position.f);                                /* normalise */
        vec3_mul_scalar(target_position.f, -child_node->dist_to_parent);  /* child points to parent */
        vec3_add_vec3(target_position.f, child_node->position.f);         /* attach to child -- this is the new target for the next segment */

        /* Calculate global rotation of parent node *
        segment_original = child_node->initial_position;
        segment_current  = child_node->position;
        vec3_sub_vec3(segment_original.f, parent_node->initial_position.f);
        vec3_sub_vec3(segment_current.f, target_position.f);
        vec3_angle(parent_node->rotation.f, segment_original.f, segment_current.f);
        quat_mul_quat(parent_node->rotation.f, parent_node->initial_rotation.f);

        * Convert global transform to local *
        inv_rotation = accumulated.rotation;
        quat_conj(inv_rotation.f);
        quat_mul_quat(parent_node->rotation.f, inv_rotation.f);
        vec3_sub_vec3(parent_node->position.f, accumulated.position.f);
        quat_rotate_vec(parent_node->position.f, inv_rotation.f);

        if (child_node->constraint != NULL)
            child_node->constraint->apply(parent_node);

        * Accumulate local rotation and translation for deeper nodes *after*
         * constraint was applied *
        accumulated_previous = accumulated;
        quat_mul_quat(accumulated.rotation.f, parent_node->rotation.f);
        vec3_add_vec3(accumulated.position.f, parent_node->position.f);

        * Convert local transform back to global *
        quat_rotate_vec(parent_node->position.f, accumulated_previous.rotation.f);
        vec3_add_vec3(parent_node->position.f, accumulated_previous.position.f);
        quat_mul_quat(parent_node->rotation.f, accumulated_previous.rotation.f);

        if (child_node->constraint != NULL)
        {
            * XXX combine this? *
            inv_rotation = parent_node->initial_rotation;
            quat_conj(inv_rotation.f);
            quat_mul_quat(parent_node->rotation.f, inv_rotation.f);

            target_position = parent_node->position;
            quat_rotate_vec(segment_original.f, parent_node->rotation.f);
            vec3_add_vec3(target_position.f, segment_original.f);
        }*/
    }

    return target_position;
}

/* ------------------------------------------------------------------------- */
static vec3_t
solve_chain_forwards(struct chain_t* chain)
{
    int node_count, node_idx;
    int average_count;
    vec3_t target_position;

    /*
     * Target position is the average of all solved child chain base positions.
     */
    vec3_set_zero(target_position.f);
    average_count = 0;
    CHAIN_FOR_EACH_CHILD(chain, child)
        vec3_t child_base_position = solve_chain_forwards(child);
        vec3_add_vec3(target_position.f, child_base_position.f);
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
        struct ik_node_t* effector_node = chain_get_node(chain, 0);
        struct ik_effector_t* effector = effector_node->effector;
        target_position = effector->_actual_target;
    }
    else
    {
        vec3_div_scalar(target_position.f, average_count);
    }

    /*
     * Iterate through each segment and apply the FABRIK algorithm.
     */
    node_count = chain_length(chain);
    for (node_idx = 0; node_idx < node_count - 1; ++node_idx)
    {
        struct ik_node_t* child_node  = chain_get_node(chain, node_idx + 0);
        struct ik_node_t* parent_node = chain_get_node(chain, node_idx + 1);

        /* move node to target */
        child_node->position = target_position;

        /* point segment to previous node and set target position to its end */
        vec3_sub_vec3(target_position.f, parent_node->position.f);        /* parent points to child */
        vec3_normalize(target_position.f);                                /* normalise */
        vec3_mul_scalar(target_position.f, -child_node->dist_to_parent);  /* child points to parent */
        vec3_add_vec3(target_position.f, child_node->position.f);         /* attach to child -- this is the new target for next iteration */
    }

    return target_position;
}

/* ------------------------------------------------------------------------- */
static void
solve_chain_backwards_with_constraints(struct chain_t* chain,
                                       vec3_t target_position,
                                       quat_t acc_rot, vec3_t acc_pos)
{
    int node_idx = chain_length(chain) - 1;

    /*
     * The base node must be set to the target position before iterating.
     * XXX: Why is this conditional?
     */
    if (node_idx > 1)
    {
        struct ik_node_t* base_node = chain_get_node(chain, node_idx);
        base_node->position = target_position;
    }

    /*
     * Iterate through each segment the other way around and apply the FABRIK
     * algorithm (starting at the base and moving down to the tip).
     */
    while (node_idx-- > 0)
    {
        struct ik_node_t* child_node  = chain_get_node(chain, node_idx + 0);
        struct ik_node_t* parent_node = chain_get_node(chain, node_idx + 1);

        /* point segment to child node and set target position to its beginning */
        vec3_sub_vec3(target_position.f, child_node->position.f);         /* child points to parent */
        vec3_normalize(target_position.f);                                /* normalise */
        vec3_mul_scalar(target_position.f, -child_node->dist_to_parent);  /* parent points to child */
        vec3_add_vec3(target_position.f, parent_node->position.f);        /* attach to parent -- this is the new target */

        /* target_position is now where the position of child_node should be. */

        /* Calculate delta rotation of parent node *
        segment_original = child_node->initial_position;
        segment_current  = target_position;
        vec3_sub_vec3(segment_original.f, parent_node->initial_position.f);
        vec3_sub_vec3(segment_current.f, parent_node->position.f);
        vec3_angle(parent_node->rotation.f, segment_original.f, segment_current.f);

        *
         * Since the initial rotation is in local space temporarily (see
         * solve() entry point on why), we now have the rotation in local space
         *
        quat_mul_quat(parent_node->rotation.f, parent_node->initial_rotation.f);

        * Convert global translation to local *
        inv_rotation = accumulated_positions.rotation;
        quat_conj(inv_rotation.f);
        vec3_sub_vec3(parent_node->position.f, accumulated_positions.position.f);
        quat_rotate_vec(parent_node->position.f, inv_rotation.f);

        if (child_node->constraint != NULL)
            child_node->constraint->apply(parent_node);

        * Accumulate local rotation and translation for deeper nodes *after*
         * constraint was applied *
        accumulated_previous = accumulated_positions;
        vec3_add_vec3(accumulated_positions.position.f, parent_node->position.f);

        * Convert local transform back to global *
        quat_rotate_vec(parent_node->position.f, accumulated_previous.rotation.f);
        vec3_add_vec3(parent_node->position.f, accumulated_previous.position.f);
        quat_mul_quat(parent_node->rotation.f, accumulated_previous.rotation.f);

        if (child_node->constraint != NULL)
        {
            * XXX combine this? *
            inv_rotation = parent_node->initial_rotation;
            quat_conj(inv_rotation.f);
            quat_mul_quat(parent_node->rotation.f, inv_rotation.f);

            target_position = parent_node->position;
            quat_rotate_vec(segment_original.f, parent_node->rotation.f);
            vec3_add_vec3(target_position.f, segment_original.f);
        }*/

        /* move node to target */
        child_node->position = target_position;
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        solve_chain_backwards_with_constraints(child, target_position, acc_rot, acc_pos);
    CHAIN_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
solve_chain_backwards(struct chain_t* chain, vec3_t target_position)
{
    int node_idx = chain_length(chain) - 1;

    /*
     * The base node must be set to the target position before iterating.
     * XXX: Why is this conditional?
     */
    if (node_idx > 1)
    {
        struct ik_node_t* base_node = chain_get_node(chain, node_idx);
        base_node->position = target_position;
    }

    /*
     * Iterate through each segment the other way around and apply the FABRIK
     * algorithm.
     */
    while (node_idx-- > 0)
    {
        struct ik_node_t* child_node  = chain_get_node(chain, node_idx + 0);
        struct ik_node_t* parent_node = chain_get_node(chain, node_idx + 1);

        /* point segment to child node and set target position to its beginning */
        vec3_sub_vec3(target_position.f, child_node->position.f); /* child points to parent */
        vec3_normalize(target_position.f);                                  /* normalise */
        vec3_mul_scalar(target_position.f, -child_node->dist_to_parent);    /* parent points to child */
        vec3_add_vec3(target_position.f, parent_node->position.f);/* attach to parent -- this is the new target */

        /* move node to target */
        child_node->position = target_position;
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        solve_chain_backwards(child, target_position);
    CHAIN_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
calculate_joint_rotations(const struct chain_t* chain);
static void
recurse_into_children(const struct chain_t* chain)
{
    /* Recurse into children chains */
    quat_t average_rotation = {{0, 0, 0, 0}};
    int average_count = 0;
    CHAIN_FOR_EACH_CHILD(chain, child)
        quat_t rotation;
        calculate_joint_rotations(child);

        /* Note: All chains *MUST* have at least two nodes */
        assert(chain_length(child) >= 2);
        rotation = chain_get_base_node(child)->rotation;

        /*
         * Averaging quaternions taken from here
         * http://wiki.unity3d.com/index.php/Averaging_Quaternions_and_Vectors
         */
        quat_normalise_sign(rotation.f);
        quat_add_quat(average_rotation.f, rotation.f);
        ++average_count;
    CHAIN_END_EACH

    /*
     * Assuming there was more than 1 child chain and assuming we aren't the
     * base node, then the child chains we just iterated must share the same
     * sub-base node (which is our tip node). Average the accumulated
     * quaternion and set this node's correct solved rotation.
     */
    if (average_count > 0 && chain_length(chain) != 0)
    {
        quat_div_scalar(average_rotation.f, average_count);
        quat_normalise(average_rotation.f);
        chain_get_tip_node(chain)->rotation = average_rotation;
    }
}

/* ------------------------------------------------------------------------- */
static void
calculate_delta_rotation_of_each_segment(const struct chain_t* chain)
{
    int node_idx;

    /*
     * Compare each segment in the input and output chain and calculate the
     * delta (!) angles. The result will be written to the output tree's
     * node->rotation field.
     *
     * NOTE: Assumes we're working on local space.
     */
    node_idx = chain_length(chain) - 1;
    while (node_idx-- > 0)
    {
        struct ik_node_t* child_node   = chain_get_node(chain, node_idx + 0);
        struct ik_node_t* parent_node = chain_get_node(chain, node_idx + 1);

        vec3_t original_segment  = child_node->initial_position;
        vec3_t solved_segment = child_node->position;
        vec3_sub_vec3(original_segment.f, parent_node->position.f);
        vec3_sub_vec3(solved_segment.f, parent_node->position.f);

        vec3_angle(parent_node->rotation.f, child_node->initial_position.f, child_node->position.f);
    }
}

/* ------------------------------------------------------------------------- */
static void
calculate_joint_rotations(const struct chain_t* chain)
{
    /*
     * Calculates the "global" (world) angles of each joint and writes them to
     * each node->rotation slot.
     *
     * The angle between the original and solved segments are calculated using
     * standard vector math (dot product). The axis of rotation is calculated
     * with the cross product. From this data, a quaternion is constructed,
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
     * At this point, all nodes have calculated their delta angles *except* for
     * the end effector nodes, which remain untouched. It makes sense to copy
     * the delta rotation of the parent node into the effector node by default.
     */
    assert(chain_length(chain) > 1);
    {
        struct ik_node_t* effector_node  = chain_get_node(chain, 0);
        struct ik_node_t* parent_node    = chain_get_node(chain, 1);
        effector_node->rotation = parent_node->rotation;
    }

    /*
     * Finally, apply initial global rotations to calculated delta rotations to
     * obtain the solved global rotations.
     */
    CHAIN_FOR_EACH_NODE(chain, node)
        quat_mul_quat(node->rotation.f, node->initial_rotation.f);
    CHAIN_END_EACH
}


/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_FABRIK_construct(struct ik_solver_t* solver)
{
    /* typical default values */
    solver->max_iterations = 20;
    solver->tolerance = 1e-3;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_FABRIK_destruct(struct ik_solver_t* solver)
{
}

/* ------------------------------------------------------------------------- */
static void
store_original_transform_for_chain(struct chain_t* chain)
{
    CHAIN_FOR_EACH_NODE(chain, node)
        node->initial_position = node->position;
        node->initial_rotation = node->rotation;
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD(chain, child)
        store_original_transform_for_chain(child);
    CHAIN_END_EACH
}
static void
store_original_transform(const struct vector_t* chain_list)
{
    VECTOR_FOR_EACH(chain_list, struct chain_t, chain)
        store_original_transform_for_chain(chain);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_FABRIK_solve(struct ik_solver_t* solver)
{
    ikret_t result = IK_OK;
    int iteration = solver->max_iterations;
    ikreal_t tolerance_squared = solver->tolerance * solver->tolerance;

    /*
     * FABRIK requires the original positions and rotations in local space to
     * calcualte joint rotations after solving.
     */
    if (solver->flags & IK_ENABLE_JOINT_ROTATIONS)
        store_original_transform(&solver->chain_list);

    /* Tree is in local space -- FABRIK needs only global node positions */
    ik_transform_chain_list(&solver->chain_list, TR_L2G | TR_TRANSLATIONS);

    while (iteration-- > 0)
    {
        /* Actual algorithm here */
        SOLVER_FOR_EACH_CHAIN(solver, chain)
            struct  ik_node_t* base_node;
            int idx;

            /* The algorithm assumes chains have at least one bone. This should
             * be asserted while building the chain trees, but it can't hurt
             * to double check */
            idx = chain_length(chain) - 1;
            assert(idx > 0);

            base_node = chain_get_node(chain, idx);

            if (solver->flags & IK_ENABLE_TARGET_ROTATIONS)
                solve_chain_forwards_with_target_rotation(chain);
            else
                solve_chain_forwards(chain);

            if (solver->flags & IK_ENABLE_CONSTRAINTS)
                solve_chain_backwards_with_constraints(chain, base_node->position, base_node->rotation, base_node->position);
            else
                solve_chain_backwards(chain, base_node->position);
        SOLVER_END_EACH

        /* Check if all effectors are within range */
        SOLVER_FOR_EACH_EFFECTOR_NODE(solver, node)
            vec3_t diff = node->position;
            vec3_sub_vec3(diff.f, node->effector->target_position.f);
            if (vec3_length_squared(diff.f) > tolerance_squared)
            {
                result = IK_RESULT_CONVERGED;
                break;
            }
        SOLVER_END_EACH
    }

    /* Transform back to local space now that solving is complete */
    ik_transform_chain_list(&solver->chain_list, TR_G2L | TR_TRANSLATIONS);

    if (solver->flags & IK_ENABLE_JOINT_ROTATIONS)
    {
        SOLVER_FOR_EACH_CHAIN(solver, chain)
            calculate_joint_rotations(chain);
        SOLVER_END_EACH
    }

    return result;
}
