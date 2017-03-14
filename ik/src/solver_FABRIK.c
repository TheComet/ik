#include "ik/bst_vector.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/solver_FABRIK.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

enum node_marking_e
{
    MARK_NONE = 0,
    MARK_SPLIT,
    MARK_SECTION
};

void
chain_construct(struct chain_t* chain);
void
chain_clear_free(struct chain_t* chain);
void
chain_destruct(struct chain_t* chain);

/* ------------------------------------------------------------------------- */
void
chain_construct(struct chain_t* chain)
{
    ordered_vector_construct(&chain->nodes, sizeof(struct ik_node_t*));
    ordered_vector_construct(&chain->children, sizeof(struct chain_t));
}

/* ------------------------------------------------------------------------- */
void
chain_clear_free(struct chain_t* chain)
{
    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child_chain)
        chain_destruct(child_chain);
    ORDERED_VECTOR_END_EACH
    ordered_vector_clear_free(&chain->children);
    ordered_vector_clear_free(&chain->nodes);
}

/* ------------------------------------------------------------------------- */
void
chain_destruct(struct chain_t* chain)
{
    chain_clear_free(chain);
}

/*!
 * @brief Breaks down the relevant nodes of the scene graph into a list of
 * chains. FABRIK can then more efficiently solve each chain individually.
 *
 * A "sub-base joint" is a node in the scene graph where at least two end
 * effector nodes eventually join together. FABRIK only works on single
 * chains of joints at a time. The end position of every sub-base joint is
 * the average of the resulting multiple positions after running FABRIK on
 * each chain. Said average position becomes the new target position for
 * the next chain connected to it.
 *
 * This algorithm finds all sub-base joints and generates chains between
 * base, sub-base joints, and end effectors. These chains are inserted into
 * the chain list. The order is such that iterating the list from the
 * beginning results in traversing the sub-base nodes breadth-last. This is
 * important.
 *
 * @note Effectors that are deactivated or invalid are ignored in this search.
 * So even though a node might share two effectors, if one of them is
 * deactivated, then the node is no longer considered a sub-base node.
 */
static int
rebuild_chain_tree(struct fabrik_t* solver);

/* ------------------------------------------------------------------------- */
struct ik_solver_t*
solver_FABRIK_create(void)
{
    struct fabrik_t* solver;

    ik_log_message("Creating FABRIK solver");

    solver = (struct fabrik_t*)MALLOC(sizeof *solver);
    if(solver == NULL)
    {
        ik_log_message("Ran out of memory in solver_FABRIK_create()");
        goto alloc_solver_filed;
    }
    memset(solver, 0, sizeof *solver);

    solver->destroy = solver_FABRIK_destroy;
    solver->rebuild_data = solver_FABRIK_rebuild_data;
    solver->solve = solver_FABRIK_solve;
    solver->reset = solver_FABRIK_reset;

    solver->max_iterations = 20;
    solver->tolerance = 1e-3;

    solver->chain_tree = (struct chain_t*)MALLOC(sizeof(struct chain_t));
    if(solver->chain_tree == NULL)
    {
        ik_log_message("Ran out of memory in solver_FABRIK_create()");
        goto alloc_chain_tree_failed;
    }
    chain_construct(solver->chain_tree);

    return (struct ik_solver_t*)solver;

    alloc_chain_tree_failed : FREE(solver);
    alloc_solver_filed      : return NULL;
}

/* ------------------------------------------------------------------------- */
void
solver_FABRIK_destroy(struct ik_solver_t* solver)
{
    struct fabrik_t* fabrik;

    ik_log_message("Destroying FABRIK solver");

    fabrik = (struct fabrik_t*)solver;
    chain_destruct(fabrik->chain_tree);
    FREE(fabrik->chain_tree);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_rebuild_data(struct ik_solver_t* solver)
{
    return rebuild_chain_tree((struct fabrik_t*)solver);
}

/* ------------------------------------------------------------------------- */
static void
solver_reset_recursive(struct chain_t* chain)
{
    ORDERED_VECTOR_FOR_EACH(&chain->nodes, struct ik_node_t*, pnode)
        (*pnode)->solved_position = (*pnode)->position;
        (*pnode)->solved_rotation = (*pnode)->rotation;
    ORDERED_VECTOR_END_EACH

    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        solver_reset_recursive(child);
    ORDERED_VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static vec3_t
solve_chain_forwards(struct chain_t* chain)
{
    int node_count, node_idx;
    vec3_t target_position = {{0, 0, 0}};
    float average_count = 0;

    /*
     * Target position is the average of all solved child chain base positions.
     * If there are no child chains, then the first node in the chain must
     * contain an effector. The target position is the effector's target
     * position.
     */
    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        const vec3_t base_position = solve_chain_forwards(child);
        vec3_add_vec3(target_position.f, base_position.f);
        ++average_count;
    ORDERED_VECTOR_END_EACH
    if(average_count != 0)
    {
        vec3_divide_scalar(target_position.f, average_count);
    }
    else
    {
        struct ik_node_t* effector_node;
        assert(ordered_vector_count(&chain->nodes) > 1);
        effector_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, 0);
        assert(effector_node->effector != NULL);
        target_position = effector_node->effector->target_position;
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
        child_node->solved_position = target_position;

        /* point segment to previous node and set target position to its end */
        vec3_sub_vec3(target_position.f, parent_node->solved_position.f); /* parent points to child */
        vec3_normalise(target_position.f);                               /* normalise */
        vec3_mul_scalar(target_position.f, -child_node->segment_length); /* child points to parent */
        vec3_add_vec3(target_position.f, child_node->solved_position.f);  /* attach to child -- this is the new target */
    }

    return target_position;
}

/* ------------------------------------------------------------------------- */
static void
solve_chain_backwards(struct chain_t* chain, vec3_t target_position)
{
    int node_idx = ordered_vector_count(&chain->nodes) - 1;

    /*
     * The base node must be set to the target position before iterating.
     */
    if(node_idx > 1)
    {
        struct ik_node_t* base_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx);
        base_node->solved_position = target_position;
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
        vec3_sub_vec3(target_position.f, child_node->solved_position.f);  /* child points to parent */
        vec3_normalise(target_position.f);                               /* normalise */
        vec3_mul_scalar(target_position.f, -child_node->segment_length); /* parent points to child */
        vec3_add_vec3(target_position.f, parent_node->solved_position.f); /* attach to parent -- this is the new target */

        /* move node to target */
        child_node->solved_position = target_position;
    }

    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        solve_chain_backwards(child, target_position);
    ORDERED_VECTOR_END_EACH
}

static void
calculate_global_angles(struct chain_t* chain)
{
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
     */
    int node_idx = ordered_vector_count(&chain->nodes) - 1;

    while(node_idx-- > 0)
    {
        ik_real cos_a, sin_a, angle, denominator;
        struct ik_node_t* child_node  = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 0);
        struct ik_node_t* parent_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx + 1);

        /* calculate original and solved segment */
        vec3_t segment_original = child_node->position;
        vec3_t segment_solved   = child_node->solved_position;
        vec3_sub_vec3(segment_original.f, parent_node->position.f);
        vec3_sub_vec3(segment_solved.f, parent_node->solved_position.f);

        /* Calculate angle between original segment and solved segment */
        denominator = 1.0 / vec3_length(segment_original.f) / vec3_length(segment_solved.f);
        cos_a = vec3_dot(segment_original.f, segment_solved.f) * denominator;
        if(cos_a < -1.0 || cos_a > 1.0)
            quat_set_identity(parent_node->solved_rotation.f);
        else
        {
            /* calculate axis of rotation and write it to the quaternion's vector section */
            parent_node->solved_rotation.vw.v = segment_original;
            vec3_cross(parent_node->solved_rotation.vw.v.f, segment_solved.f);
            vec3_normalise(parent_node->solved_rotation.f);

            /* quaternion's vector needs to be weighted with sin_a */
            angle = acos(cos_a);
            cos_a = cos(angle * 0.5);
            sin_a = sin(angle * 0.5);
            vec3_mul_scalar(parent_node->solved_rotation.f, sin_a);
            parent_node->solved_rotation.q.w = cos_a;

            /*
             * Apply initial global rotation to calculated delta rotation to
             * obtain the solved global rotation.
             */
            quat_mul(parent_node->solved_rotation.f, parent_node->rotation.f);
        }
    }

    /* Recursive into children chains */
    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        calculate_global_angles(child);
    ORDERED_VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
iterate_tree_recursive(struct ik_solver_t* solver, struct chain_t* chain)
{
    int node_idx;

    /*
     * Apply from base to tips (breadth first), omit the base node for each
     * chain because that would be redundant (it is shared with the tip of the
     * parent chain).
     */
    node_idx = ordered_vector_count(&chain->nodes);
    while(node_idx-- > 0)
    {
        struct ik_node_t* node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx);
        solver->apply_result(node);
    }

    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        iterate_tree_recursive(solver, child);
    ORDERED_VECTOR_END_EACH
}
void
ik_solver_iterate_tree(struct ik_solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    iterate_tree_recursive(solver, fabrik->chain_tree);
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_solve(struct ik_solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    int iteration = solver->max_iterations;

    solver_reset_recursive(fabrik->chain_tree);

    while(iteration--)
    {
        ORDERED_VECTOR_FOR_EACH(&fabrik->chain_tree->children, struct chain_t, chain)
            vec3_t root_position;
            assert(ordered_vector_count(&chain->nodes) > 1);
            root_position = (*(struct ik_node_t**)ordered_vector_get_element(&chain->nodes,
                    ordered_vector_count(&chain->nodes) - 1))->position;

            solve_chain_forwards(chain);
            solve_chain_backwards(chain, root_position);
        ORDERED_VECTOR_END_EACH
    }

    calculate_global_angles(fabrik->chain_tree);
    ik_solver_iterate_tree(solver);

    return 0;
}

/* ------------------------------------------------------------------------- */
void
solver_FABRIK_reset(struct ik_solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    solver_reset_recursive(fabrik->chain_tree);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
int
mark_involved_nodes(struct fabrik_t* solver, struct bstv_t* involved_nodes)
{
    /*
     * Traverse the chain of parents starting at each effector node and ending
     * at the root node of the tree and mark every node on the way. Each
     * effector specifies a maximum chain length, which means it's possible
     * that we won't hit the root node.
     */
    struct ordered_vector_t* effector_nodes_list = &solver->effector_nodes_list;
    ORDERED_VECTOR_FOR_EACH(effector_nodes_list, struct ik_node_t*, p_effector_node)

        /*
         * Set up chain length counter. If the chain length is 0 then it is
         * infinitely long. Set the counter to -1 in this case to skip the
         * escape condition.
         */
        int chain_length_counter;
        struct ik_node_t* node = *p_effector_node;
        assert(node->effector != NULL);
        chain_length_counter = node->effector->chain_length == 0 ? -1 : (int)node->effector->chain_length;

        /*
         * Mark nodes that are at the base of the chain differently, so the
         * chains can be split correctly later. Section markings will overwrite
         * break markings.
         */
        for(; node != NULL; node = node->parent)
        {
            enum node_marking_e* current_marking;
            enum node_marking_e marking = MARK_SECTION;
            if(chain_length_counter == 0)
                marking = MARK_SPLIT;

            current_marking = (enum node_marking_e*)bstv_find_ptr(involved_nodes, node->guid);
            if(current_marking == NULL)
            {
                if(bstv_insert(involved_nodes, node->guid, (void*)(intptr_t)marking) < 0)
                {
                    ik_log_message("Ran out of memory while marking involved nodes");
                    return -1;
                }
            }
            else
            {
                if(chain_length_counter != 0)
                    *current_marking = marking;
            }

            if(chain_length_counter-- == 0)
                break;
        }
    ORDERED_VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
recursively_build_chain_tree(struct chain_t* chain_current,
                             struct ik_node_t* node_base,
                             struct ik_node_t* node_current,
                             struct bstv_t* involved_nodes)
{
    int marked_children_count;
    struct ik_node_t* child_node_base = node_base;
    struct chain_t* child_chain = chain_current;

    /* can remove the mark from the set to speed up future checks */
    enum node_marking_e marking =
        (enum node_marking_e)(intptr_t)bstv_erase(involved_nodes, node_current->guid);

    switch(marking)
    {
        /*
         * If this node was marked as the base of a chain then split the chain at
         * this point by moving the pointer to the base node down the tree to us.
         */
        case MARK_SPLIT:
            child_node_base = node_current;
            break;
        /*
         * If this node is not marked at all, cut off any previous chain but
         * continue (fall through) as if a section was marked. It's possible
         * that there are isolated chains somewhere further down the tree.
         */
        case MARK_NONE:
            node_base = node_current;

        case MARK_SECTION:
            /*
             * If the current node has at least two children marked as sections
             * or if the current node is an effector node, but only if the base
             * node is not equal to this node (that is, we need to avoid chains
             * that would have less than 2 nodes), then we must also split the
             * chain at this point.
             */
            marked_children_count = 0;
            BSTV_FOR_EACH(&node_current->children, struct ik_node_t, child_guid, child)
                if((enum node_marking_e)(intptr_t)bstv_find(involved_nodes, child_guid) == MARK_SECTION)
                    if(++marked_children_count == 2)
                        break;
            BSTV_END_EACH
            if((marked_children_count == 2 || node_current->effector != NULL) && node_current != node_base)
            {
                /*
                 * Emplace a chain object into the current chain's vector of children
                 * and initialise it.
                 */
                struct ik_node_t* node;
                child_chain = ordered_vector_push_emplace(&chain_current->children);
                if(child_chain == NULL)
                    return -1;
                chain_construct(child_chain);

                /*
                 * Add points to all nodes that are part of this chain into the chain's
                 * list, starting with the end node.
                 */
                for(node = node_current; node != node_base; node = node->parent)
                    ordered_vector_push(&child_chain->nodes, &node);
                ordered_vector_push(&child_chain->nodes, &node_base);

                /*
                 * Update the base node to be this node so deeper chains are built back
                 * to this node
                 */
                child_node_base = node_current;
            }
            break;
    }

    /* Recurse into children of the current node. */
    BSTV_FOR_EACH(&node_current->children, struct ik_node_t, child_guid, child_node)
        if(recursively_build_chain_tree(
                child_chain,
                child_node_base,
                child_node,
                involved_nodes) < 0)
            return -1;
    BSTV_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
static void
compute_segment_lengths(struct chain_t* chain)
{
    int last_idx = ordered_vector_count(&chain->nodes) - 1;
    while(last_idx-- > 0)
    {
        struct ik_node_t* child_node =
            *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 0);
        struct ik_node_t* parent_node =
            *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 1);

        vec3_t diff = child_node->position;
        vec3_sub_vec3(diff.f, parent_node->position.f);
        child_node->segment_length = vec3_length(diff.f);
    }

    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        compute_segment_lengths(child);
    ORDERED_VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static int
count_chains(struct chain_t* chain)
{
    int counter = 1;
    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        counter += count_chains(child);
    ORDERED_VECTOR_END_EACH
    return counter;
}

/* ------------------------------------------------------------------------- */
#if IK_DOT_OUTPUT == ON
static void
dump_chain(struct chain_t* chain, FILE* fp)
{
    int last_idx = ordered_vector_count(&chain->nodes) - 1;
    if(last_idx > 0)
    {
        fprintf(fp, "    %d [shape=record];\n",
            (*(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, 0))->guid);
        fprintf(fp, "    %d [shape=record];\n",
            (*(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx))->guid);
    }

    while(last_idx-- > 0)
    {
        fprintf(fp, "    %d -- %d [color=\"1.0 0.5 1.0\"];\n",
            (*(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 0))->guid,
            (*(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 1))->guid);
    }

    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        dump_chain(child, fp);
    ORDERED_VECTOR_END_EACH
}
static void
dump_node(struct ik_node_t* node, FILE* fp)
{
    if(node->effector != NULL)
        fprintf(fp, "    %d [color=\"0.6 0.5 1.0\"];\n", node->guid);
    BSTV_FOR_EACH(&node->children, struct ik_node_t, guid, child)
        fprintf(fp, "    %d -- %d;\n", node->guid, guid);
        dump_node(child, fp);
    BSTV_END_EACH
}
static void
dump_to_dot(struct ik_node_t* node, struct chain_t* chain, const char* file_name)
{
    FILE* fp = fopen(file_name, "w");
    if(fp == NULL)
        return;

    fprintf(fp, "graph chain_tree {\n");
    dump_node(node, fp);
    dump_chain(chain, fp);
    fprintf(fp, "}\n");

    fclose(fp);
}
#endif

/* ------------------------------------------------------------------------- */
static int
rebuild_chain_tree(struct fabrik_t* solver)
{
    struct bstv_t involved_nodes;
    int involved_nodes_count;
#if IK_DOT_OUTPUT == ON
    char buffer[20];
    static int file_name_counter = 0;
#endif

    /*
     * Build a set of all nodes that are in a direct path with all of the
     * effectors.
     */
    bstv_construct(&involved_nodes);
    if(mark_involved_nodes(solver, &involved_nodes) < 0)
        goto mark_involved_nodes_failed;
    involved_nodes_count = bstv_count(&involved_nodes);

    /*
     * The user can choose to set the root node as a chain terminator (default)
     * or choose to exclude the root node, in which case each immediate child
     * of the tree is a chain terminator. In this case we need to build the
     * chain tree for each child individually.
     */
    chain_clear_free(solver->chain_tree);
    switch(solver->build_mode)
    {
        case SOLVER_INCLUDE_ROOT:
        {
            struct ik_node_t* root = solver->tree;
            recursively_build_chain_tree(solver->chain_tree, root, root, &involved_nodes);
            break;
        }

        case SOLVER_EXCLUDE_ROOT:
        {
            BSTV_FOR_EACH(&solver->tree->children, struct ik_node_t, guid, child)
                recursively_build_chain_tree(solver->chain_tree, child, child, &involved_nodes);
            BSTV_END_EACH
        }
    }

    /* Pre-compute offsets for each node in the chain tree in relation to their
     * parents */
    compute_segment_lengths(solver->chain_tree);

    /* DEBUG: Save chain tree to DOT */
#if IK_DOT_OUTPUT == ON
    sprintf(buffer, "tree%d.dot", file_name_counter++);
    dump_to_dot(root, solver->chain_tree, buffer);
#endif

    ik_log_message("There are %d effector(s) involving %d node(s). %d chain(s) were created",
                   ordered_vector_count(&solver->effector_nodes_list),
                   involved_nodes_count,
                   count_chains(solver->chain_tree) - 1); /* don't count root chain which always exists */

    bstv_clear_free(&involved_nodes);

    return 0;

    mark_involved_nodes_failed : bstv_clear_free(&involved_nodes);
    return -1;
}
