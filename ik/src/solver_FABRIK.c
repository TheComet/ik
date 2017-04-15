#include "ik/bst_vector.h"
#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/solver_FABRIK.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_construct(struct ik_solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;

    /* set up derived functions */
    fabrik->destruct = solver_FABRIK_destruct;
    fabrik->solve = solver_FABRIK_solve;

    /* typical default values */
    fabrik->max_iterations = 20;
    fabrik->tolerance = 1e-3;

    return 0;
}

/* ------------------------------------------------------------------------- */
void
solver_FABRIK_destruct(struct ik_solver_t* solver)
{
}

/* ------------------------------------------------------------------------- */
static void
determine_target_data_from_effector(struct ik_chain_t* chain, vec3_t* target_position)
{
    /* Extract effector node and get its effector object */
    struct ik_node_t* effector_node;
    struct ik_effector_t* effector;
    assert(ordered_vector_count(&chain->nodes) > 1);
    effector_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, 0);
    assert(effector_node->effector != NULL);
    effector = effector_node->effector;

    /* lerp using effector weight to get actual target position */
    *target_position = effector->target_position;
    vec3_sub_vec3(target_position->f, effector_node->initial_position.f);
    vec3_mul_scalar(target_position->f, effector->weight);
    vec3_add_vec3(target_position->f, effector_node->initial_position.f);

    /* Fancy algorithm using nlerp, makes transitions look more natural */
    if(effector->flags & EFFECTOR_WEIGHT_NLERP && effector->weight < 1.0)
    {
        ik_real distance_to_target;
        vec3_t base_to_effector;
        vec3_t base_to_target;
        struct ik_node_t* base_node;

        /* Need distance from base node to target and base to effector node */
        base_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes,
                ordered_vector_count(&chain->nodes) - 1);
        base_to_effector = effector_node->initial_position;
        base_to_target = effector->target_position;
        vec3_sub_vec3(base_to_effector.f, base_node->initial_position.f);
        vec3_sub_vec3(base_to_target.f, base_node->initial_position.f);

        /* The effective distance is a lerp between these two distances */
        distance_to_target = vec3_length(base_to_target.f) * effector->weight;
        distance_to_target += vec3_length(base_to_effector.f) * (1.0 - effector->weight);

        /* nlerp the target position by pinning it to the base node */
        vec3_sub_vec3(target_position->f, base_node->initial_position.f);
        vec3_normalise(target_position->f);
        vec3_mul_scalar(target_position->f, distance_to_target);
        vec3_add_vec3(target_position->f, base_node->initial_position.f);
    }
}

/* ------------------------------------------------------------------------- */
struct position_direction_t
{
    vec3_t position;
    vec3_t direction;
};
static struct position_direction_t
solve_chain_forwards_with_target_rotation(struct ik_chain_t* chain)
{
    int node_count, node_idx;
    int average_count;
    struct position_direction_t target;

    vec3_set_zero(target.position.f);

    /*
     * Target position is the average of all solved child chain base positions.
     */
    average_count = 0;
    ORDERED_VECTOR_FOR_EACH(&chain->children, struct ik_chain_t, child)
        struct position_direction_t child_posdir = solve_chain_forwards_with_target_rotation(child);
        vec3_add_vec3(target.position.f, child_posdir.position.f);
        vec3_add_vec3(target.direction.f, child_posdir.direction.f);
        ++average_count;
    ORDERED_VECTOR_END_EACH

    /*
     * If there are no child chains, then the first node in the chain must
     * contain an effector. The target position is the effector's target
     * position. Otherwise, average the data we've been accumulating from the
     * child chains.
     */
    if(average_count == 0)
    {
        struct ik_node_t* effector_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, 0);
        struct ik_effector_t* effector = effector_node->effector;
        determine_target_data_from_effector(chain, &target.position);

        /* TODO This "global direction" could be made configurable if needed */
        target.direction.v.x = 0.0;
        target.direction.v.y = 0.0;
        target.direction.v.z = 1.0;
        quat_rotate_vec(target.direction.f, effector->target_rotation.f);
    }
    else
    {
        ik_real div = 1.0 / average_count;
        vec3_mul_scalar(target.position.f, div);
        vec3_mul_scalar(target.direction.f, div);
    }

    /*
     * Iterate through each segment and apply the FABRIK algorithm.
     */
    node_count = ordered_vector_count(&chain->nodes);
    for(node_idx = 0; node_idx < node_count - 1; ++node_idx)
    {
        struct ik_node_t* child_node  = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 0);
        struct ik_node_t* parent_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 1);

        /* move node to target */
        child_node->position = target.position;

        /* lerp direction vector and segment vector */
        vec3_sub_vec3(target.position.f, target.direction.f);
        vec3_sub_vec3(target.position.f, parent_node->position.f);
        vec3_mul_scalar(target.position.f, parent_node->rotation_weight);
        vec3_add_vec3(target.position.f, parent_node->position.f);

        vec3_sub_vec3(target.position.f, child_node->position.f);
        vec3_normalise(target.position.f);
        vec3_mul_scalar(target.position.f, child_node->segment_length);
        vec3_add_vec3(target.position.f, child_node->position.f);
    }

    return target;
}

/* ------------------------------------------------------------------------- */
static vec3_t
solve_chain_forwards(struct ik_chain_t* chain)
{
    int node_count, node_idx;
    int average_count;
    vec3_t target_position = {{0, 0, 0}};

    /*
     * Target position is the average of all solved child chain base positions.
     */
    average_count = 0;
    ORDERED_VECTOR_FOR_EACH(&chain->children, struct ik_chain_t, child)
        vec3_t child_base_position = solve_chain_forwards(child);
        vec3_add_vec3(target_position.f, child_base_position.f);
        ++average_count;
    ORDERED_VECTOR_END_EACH

    /*
     * If there are no child chains, then the first node in the chain must
     * contain an effector. The target position is the effector's target
     * position. Otherwise, average the data we've been accumulating from the
     * child chains.
     */
    if(average_count == 0)
        determine_target_data_from_effector(chain, &target_position);
    else
        vec3_div_scalar(target_position.f, average_count);

    /*
     * Iterate through each segment and apply the FABRIK algorithm.
     */
    node_count = ordered_vector_count(&chain->nodes);
    for(node_idx = 0; node_idx < node_count - 1; ++node_idx)
    {
        struct ik_node_t* child_node  = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 0);
        struct ik_node_t* parent_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 1);

        /* move node to target */
        child_node->position = target_position;

        /* point segment to previous node and set target position to its end */
        vec3_sub_vec3(target_position.f, parent_node->position.f);        /* parent points to child */
        vec3_normalise(target_position.f);                                /* normalise */
        vec3_mul_scalar(target_position.f, -child_node->segment_length);  /* child points to parent */
        vec3_add_vec3(target_position.f, child_node->position.f);         /* attach to child -- this is the new target */
    }

    return target_position;
}

/* ------------------------------------------------------------------------- */
void
solve_chain_backwards(struct ik_chain_t* chain, vec3_t target_position)
{
    int node_idx = ordered_vector_count(&chain->nodes) - 1;

    /*
     * The base node must be set to the target position before iterating.
     */
    if(node_idx > 1)
    {
        struct ik_node_t* base_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx);
        base_node->position = target_position;
    }

    /*
     * Iterate through each segment the other way around and apply the FABRIK
     * algorithm.
     */
    while(node_idx-- > 0)
    {
        struct ik_node_t* child_node  = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 0);
        struct ik_node_t* parent_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 1);

        /* point segment to child node and set target position to its beginning */
        vec3_sub_vec3(target_position.f, child_node->position.f);         /* child points to parent */
        vec3_normalise(target_position.f);                                /* normalise */
        vec3_mul_scalar(target_position.f, -child_node->segment_length);  /* parent points to child */
        vec3_add_vec3(target_position.f, parent_node->position.f);        /* attach to parent -- this is the new target */

        /* move node to target */
        child_node->position = target_position;
    }

    ORDERED_VECTOR_FOR_EACH(&chain->children, struct ik_chain_t, child)
        solve_chain_backwards(child, target_position);
    ORDERED_VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
calculate_global_rotations(struct ik_chain_t* chain);

static void
calculate_global_rotations_of_children(struct ik_chain_t* chain)
{
    int average_count;
    quat_t average_rotation = {{0, 0, 0, 0}};

    /* Recurse into children chains */
    average_count = 0;
    ORDERED_VECTOR_FOR_EACH(&chain->children, struct ik_chain_t, child)
        quat_t rotation;
        calculate_global_rotations(child);

        /* Note: All chains that aren't the root chain *MUST* have at least two nodes */
        assert(ordered_vector_count(&child->nodes) >= 2);
        rotation = (*(struct ik_node_t**)
                ordered_vector_get_element(&child->nodes,
                    ordered_vector_count(&child->nodes) - 1))->rotation;

        /*
         * Averaging quaternions taken from here
         * http://wiki.unity3d.com/index.php/Averaging_Quaternions_and_Vectors
         */
        quat_normalise_sign(rotation.f);
        quat_add_quat(average_rotation.f, rotation.f);
        ++average_count;
    ORDERED_VECTOR_END_EACH

    /*
     * Assuming there was more than 1 child chain and assuming we aren't the
     * root node, then the child chains we just iterated must share the same
     * base node as our tip node. Average the accumulated quaternion and set
     * this node's correct solved rotation.
     */
    if(average_count > 0 && ordered_vector_count(&chain->nodes) != 0)
    {
        quat_div_scalar(average_rotation.f, average_count);
        quat_normalise(average_rotation.f);
        (*(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, 0))
            ->rotation = average_rotation;
    }
}

/* ------------------------------------------------------------------------- */
static void
calculate_delta_rotation_of_each_segment(struct ik_chain_t* chain)
{
    int node_idx;

    /*
     * Calculate all of the delta angles of the joints. The resulting delta (!)
     * angles will be written to node->rotation
     */
    node_idx = ordered_vector_count(&chain->nodes) - 1;
    while(node_idx-- > 0)
    {
        ik_real cos_a, sin_a, angle, denominator;
        struct ik_node_t* child_node  = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 0);
        struct ik_node_t* parent_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 1);

        /* calculate vectors for original and solved segments */
        vec3_t segment_original = child_node->initial_position;
        vec3_t segment_solved   = child_node->position;
        vec3_sub_vec3(segment_original.f, parent_node->initial_position.f);
        vec3_sub_vec3(segment_solved.f, parent_node->position.f);

        /*
         * Calculate angle between original segment and solved segment. If the
         * angle is 0 or 180, we don't do anything. The solved rotation is
         * initially set to the original rotation.
         */
        denominator = 1.0 / vec3_length(segment_original.f) / vec3_length(segment_solved.f);
        cos_a = vec3_dot(segment_original.f, segment_solved.f) * denominator;
        if(cos_a >= -1.0 && cos_a <= 1.0)
        {
            /* calculate axis of rotation and write it to the quaternion's vector section */
            parent_node->rotation.vw.v = segment_original;
            vec3_cross(parent_node->rotation.vw.v.f, segment_solved.f);
            vec3_normalise(parent_node->rotation.f);

            /* quaternion's vector needs to be weighted with sin_a */
            angle = acos(cos_a);
            cos_a = cos(angle * 0.5);
            sin_a = sin(angle * 0.5);
            vec3_mul_scalar(parent_node->rotation.f, sin_a);
            parent_node->rotation.q.w = cos_a;
        }
        else
        {
            /* Important! otherwise garbage happens when applying initial rotations */
            quat_set_identity(parent_node->rotation.f);
        }
    }

}

/* ------------------------------------------------------------------------- */
static void
calculate_global_rotations(struct ik_chain_t* chain)
{
    int node_idx;

    /*
     * Calculates the "global" (world) angles of each joint and writes them to
     * each node->solved_rotation slot.
     *
     * The angle between the original and solved segments are calculated using
     * standard vector math (dot product). The axis of rotation is calculated
     * with the cross product. From this data, a quaternion is constructed,
     * describing this delta rotation. Finally, in order to make the rotations
     * global instead of relative, the delta rotation is multiplied with
     * node->rotation, which should be a quaternion describing the node's
     * global rotation in the unsolved tree.
     *
     * The rotation of the base joint in the chain is returned so it can be
     * averaged by parent chains.
     */

    calculate_global_rotations_of_children(chain);
    calculate_delta_rotation_of_each_segment(chain);

    /*
     * At this point, all nodes have calculated their delta angles *except* for
     * the end effector nodes, which remain untouched. It makes sense to copy
     * the delta rotation of the parent node into the effector node by default.
     */
    node_idx = ordered_vector_count(&chain->nodes);
    if (node_idx > 1)
    {
        struct ik_node_t* effector_node  = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, 0);
        struct ik_node_t* parent_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, 1);
        effector_node->rotation.q = parent_node->rotation.q;
    }

    /*
     * Finally, apply initial global rotations to calculated delta rotations to
     * obtain the solved global rotations.
     */
    ORDERED_VECTOR_FOR_EACH(&chain->nodes, struct ik_node_t*, pnode)
        struct ik_node_t* node = *pnode;
        quat_mul_quat(node->rotation.f, node->initial_rotation.f);
    ORDERED_VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_solve(struct ik_solver_t* solver)
{
    int result = 0;
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    int iteration = solver->max_iterations;
    ik_real tolerance_squared = solver->tolerance * solver->tolerance;

    while(iteration-- > 0)
    {
        vec3_t root_position;

        /* Actual algorithm here */
        ORDERED_VECTOR_FOR_EACH(&fabrik->chain_tree->children, struct ik_chain_t, chain)

            assert(ordered_vector_count(&chain->nodes) > 1);
            root_position = (*(struct ik_node_t**)ordered_vector_get_element(&chain->nodes,
                    ordered_vector_count(&chain->nodes) - 1))->initial_position;

            if(solver->flags & SOLVER_CALCULATE_TARGET_ROTATIONS)
                solve_chain_forwards_with_target_rotation(chain);
            else
                solve_chain_forwards(chain);

            solve_chain_backwards(chain, root_position);
        ORDERED_VECTOR_END_EACH

        /* apply constraints */
        if(solver->apply_constraints != NULL)
        {
            if(solver->flags & SOLVER_CALCULATE_CONSTRAINT_ROTATIONS)
                calculate_global_rotations(fabrik->chain_tree);

            if(solver->flags & SOLVER_CONSTRAINT_SPACE_GLOBAL)
                solver->apply_constraints(solver);
            else
            {
                ik_node_global_to_local(solver->tree);
                solver->apply_constraints(solver);
                ik_node_local_to_global(solver->tree);
            }
        }

        /* Check if all effectors are within range */
        ORDERED_VECTOR_FOR_EACH(&fabrik->effector_nodes_list, struct ik_node_t*, pnode)
            vec3_t diff = (*pnode)->position;
            vec3_sub_vec3(diff.f, (*pnode)->effector->target_position.f);
            if(vec3_length_squared(diff.f) > tolerance_squared)
            {
                result = 1; /* converged */
                break;
            }
        ORDERED_VECTOR_END_EACH
    }

    if(solver->flags & SOLVER_CALCULATE_FINAL_ROTATIONS)
        calculate_global_rotations(fabrik->chain_tree);

    return result;
}
