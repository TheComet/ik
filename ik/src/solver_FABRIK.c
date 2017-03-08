#include "ik/bst_vector.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/solver_FABRIK.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct chain_t
{
    struct ordered_vector_t nodes;  /* list of node_t objects */
    struct vector3_t* base_position;
    struct vector3_t* end_position;
};

enum node_marking_e
{
    MARK_NONE = 0,
    MARK_SPLIT,
    MARK_SECTION
};

void
clear_chain_list(struct ordered_vector_t* chain_list);

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
rebuild_chain_list(struct fabrik_t* solver);

/* ------------------------------------------------------------------------- */
struct solver_t*
solver_FABRIK_create(void)
{
    struct fabrik_t* solver = (struct fabrik_t*)MALLOC(sizeof *solver);
    if(solver == NULL)
        return NULL;
    memset(solver, 0, sizeof *solver);

    solver->base.solver.private_.destroy = solver_FABRIK_destroy;
    solver->base.solver.private_.rebuild_data = solver_FABRIK_rebuild_data;
    solver->base.solver.private_.solve = solver_FABRIK_solve;

    solver->base.solver.max_iterations = 20;
    solver->base.solver.tolerance = 1e-4;

    ordered_vector_construct(&solver->base.fabrik.chain_list, sizeof(struct chain_t));
    return (struct solver_t*)solver;
}

/* ------------------------------------------------------------------------- */
void
solver_FABRIK_destroy(struct solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    clear_chain_list(&fabrik->base.fabrik.chain_list);
    ordered_vector_clear_free(&fabrik->base.fabrik.chain_list);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_rebuild_data(struct solver_t* solver)
{
    return rebuild_chain_list((struct fabrik_t*)solver);
}

static struct vector3_t
solve_chain_forwards(struct chain_t* chain)
{
    struct node_t* effector_node = ordered_vector_get_element(&chain->nodes, 0);
    struct vector3_t target = effector_node->effector->target_position;
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_solve(struct solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    int iteration = solver->max_iterations;
    while(iteration--)
    {
        ORDERED_VECTOR_FOR_EACH(&fabrik->base.fabrik.chain_list, struct chain_t, chain)
            solve_chain_forwards(chain);
        ORDERED_VECTOR_END_EACH
    }

    return -1;
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
    struct ordered_vector_t* effector_nodes_list = &solver->base.solver.private_.effector_nodes_list;
    ORDERED_VECTOR_FOR_EACH(effector_nodes_list, struct node_t*, p_effector_node)

        /*
         * Set up chain length counter. If the chain length is 0 then it is
         * infinitely long. Set the counter to -1 in this case to skip the
         * escape condition.
         */
        int chain_length_counter;
        struct node_t* node = *p_effector_node;
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
void
clear_chain_list(struct ordered_vector_t* chain_list)
{
    struct node_t* previous_chain_end = NULL;
    ORDERED_VECTOR_FOR_EACH_R(chain_list, struct chain_t, chain)
        /*
         * Some chains share a common vector3_t object with their parents or
         * children, so we must be careful not to call free() twice on the same
         * object.
         *
         * The strategy used here is to begin with the root chain and work our
         * way down the tree of chains. The vector3_t objects at the end of
         * each chain can be freed without any danger. The vector3_t object
         * at the base is freed on the first encounter, then skipped for every
         * proceeding chain that shares this base with a parent chain's end.
         */
        uint32_t node_count = ordered_vector_count(&chain->nodes);
        struct node_t* chain_beginning = ordered_vector_get_element(&chain->nodes, node_count - 1);
        if(chain_beginning != previous_chain_end)
            FREE(chain->base_position);
        FREE(chain->end_position);
        previous_chain_end = ordered_vector_get_element(&chain->nodes, 0);;

        ordered_vector_clear_free(&chain->nodes);
    ORDERED_VECTOR_END_EACH

    ordered_vector_clear(chain_list);
}

/* ------------------------------------------------------------------------- */
struct chain_t*
recursively_build_chain_list(struct ordered_vector_t* chain_list,
                             struct bstv_t* involved_nodes,
                             struct node_t* chain_beginning,
                             struct node_t* current_node,
                             struct chain_t* previous_chain)
{
    int marked_children_count;
    struct chain_t* chain;
    struct node_t* next_chain_beginning = chain_beginning;
    struct node_t* previous_chain_beginning = NULL;

    /*
     * If this node is not marked at all, cut off any previous chain and
     * recursive into children. It's possible that there are isolated chains
     * somewhere further down the tree.
     */
    enum node_marking_e marking =
        (enum node_marking_e)(intptr_t)bstv_erase(involved_nodes, current_node->guid);
    if(marking == MARK_NONE)
    {
        BSTV_FOR_EACH(&current_node->children, struct node_t, child_guid, child)
            recursively_build_chain_list(chain_list, involved_nodes, child, child, NULL);
        BSTV_END_EACH
        return NULL;
    }

    /*
     * If this node was marked as the base of a chain then split the chain at
     * this point.
     */
    if(marking == MARK_SPLIT)
        next_chain_beginning = current_node;

    /*
     * If the current node has at least two children marked as sections, then
     * we must also split the chain at this point.
     */
    marked_children_count = 0;
    BSTV_FOR_EACH(&current_node->children, struct node_t, child_guid, child)
        if((enum node_marking_e)(intptr_t)bstv_find(involved_nodes, child_guid) == MARK_SECTION)
            if(++marked_children_count == 2)
            {
                next_chain_beginning = current_node;
                break;
            }
    BSTV_END_EACH

    /*
     * Recurse into children of the current node.
     */
    BSTV_FOR_EACH(&current_node->children, struct node_t, child_guid, child)
        previous_chain = recursively_build_chain_list(
                chain_list,
                involved_nodes,
                next_chain_beginning,
                child,
                previous_chain);
    BSTV_END_EACH

    /*
     * If the current node is not a sub-base node and is also not a leaf node,
     * that is, it has exactly one marked child, then there is no need to
     * create a new chain.
     */
    if(marked_children_count == 1)
        return previous_chain;

    /* Avoid creating chains that only have one node */
    if(current_node == chain_beginning)
        return previous_chain;

    if(previous_chain != NULL)
    {
        uint32_t node_count = ordered_vector_count(&previous_chain->nodes);
        previous_chain_beginning = ordered_vector_get_element(&previous_chain->nodes, node_count - 1);
    }

    chain = ordered_vector_push_emplace(chain_list);
    if(chain == NULL)
        return NULL;

    if(previous_chain_beginning == current_node)
        chain->end_position = previous_chain->base_position;
    else
        chain->end_position = MALLOC(sizeof(struct vector3_t));

    if(previous_chain_beginning == chain_beginning)
        chain->base_position = previous_chain->base_position;
    else
        chain->base_position = MALLOC(sizeof(struct vector3_t));

    ordered_vector_construct(&chain->nodes, sizeof(struct node_t*));
    for(; current_node != chain_beginning; current_node = current_node->parent)
        ordered_vector_push(&chain->nodes, &current_node);
    ordered_vector_push(&chain->nodes, &chain_beginning);

    return chain;
}

/* ------------------------------------------------------------------------- */
static void
recursively_dump_tree_to_dot(FILE* fp, struct node_t* node)
{
    BSTV_FOR_EACH(&node->children, struct node_t, guid, child)
        fprintf(fp, "    %d -- %d;\n", node->guid, guid);
        recursively_dump_tree_to_dot(fp, child);
    BSTV_END_EACH
}

/* ------------------------------------------------------------------------- */
void
dump_chain_list_to_dot(struct node_t* tree, struct ordered_vector_t* chain_list, const char* file_name)
{
    FILE* fp = fopen(file_name, "w");
    if(fp == NULL)
    {
        ik_log_message("Failed to open file %s", file_name);
        return;
    }

    fprintf(fp, "graph graphname {\n");
    recursively_dump_tree_to_dot(fp, tree);
    {
        float hue_step = 1.0f / ordered_vector_count(chain_list);
        float hue = 0.0f;
        ORDERED_VECTOR_FOR_EACH(chain_list, struct chain_t, chain)
            uint32_t chain_len = ordered_vector_count(&chain->nodes);
            struct node_t** chain_start = ordered_vector_get_element(&chain->nodes, chain_len - 1);
            struct node_t** chain_end = ordered_vector_get_element(&chain->nodes, 0);
            fprintf(fp, "    %d [shape=record];\n", (*chain_start)->guid);
            fprintf(fp, "    %d [shape=record];\n", (*chain_end)->guid);
            ORDERED_VECTOR_FOR_EACH(&chain->nodes, struct node_t*, node)
                if((*node)->effector == NULL)
                    fprintf(fp, "    %d [color=\"%f 0.2 1.0\"];\n", (*node)->guid, hue);
                else
                    fprintf(fp, "    %d [color=\"1.0 1.0 1.0\"];\n", (*node)->guid);
            ORDERED_VECTOR_END_EACH
            hue += hue_step;
        ORDERED_VECTOR_END_EACH
    }
    fprintf(fp, "}\n");

    fclose(fp);
}


/* ------------------------------------------------------------------------- */
static int
rebuild_chain_list(struct fabrik_t* solver)
{
    struct bstv_t involved_nodes;
    struct ordered_vector_t* chain_list = &solver->base.fabrik.chain_list;
    struct node_t* root = solver->base.solver.private_.tree;

    bstv_construct(&involved_nodes);
    if(mark_involved_nodes(solver, &involved_nodes) < 0)
        goto mark_involved_nodes_failed;

    ik_log_message("There are %d involved node(s)", bstv_count(&involved_nodes));

    clear_chain_list(chain_list);
    recursively_build_chain_list(chain_list, &involved_nodes, root, root, NULL);
    dump_chain_list_to_dot(root, chain_list, "tree.dot");

    ik_log_message("There are %d effector(s)",
                   ordered_vector_count(&solver->base.solver.private_.effector_nodes_list));
    ik_log_message("%d chain(s) were created",
                   ordered_vector_count(chain_list));

    bstv_clear_free(&involved_nodes);

    return 0;

    mark_involved_nodes_failed : bstv_clear_free(&involved_nodes);
    return -1;
}
