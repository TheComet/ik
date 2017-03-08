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
    struct ordered_vector_t nodes;    /* list of node_t* references */
    struct ordered_vector_t children; /* list of chain_t objects */
};

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
    ordered_vector_construct(&chain->nodes, sizeof(struct node_t*));
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
struct solver_t*
solver_FABRIK_create(void)
{
    struct fabrik_t* solver = (struct fabrik_t*)MALLOC(sizeof *solver);
    if(solver == NULL)
        goto alloc_solver_filed;
    memset(solver, 0, sizeof *solver);

    solver->destroy = solver_FABRIK_destroy;
    solver->rebuild_data = solver_FABRIK_rebuild_data;
    solver->solve = solver_FABRIK_solve;

    solver->max_iterations = 20;
    solver->tolerance = 1e-4;

    solver->chain_tree = (struct chain_t*)MALLOC(sizeof(struct chain_t));
    if(solver->chain_tree == NULL)
        goto alloc_chain_tree_failed;
    chain_construct(solver->chain_tree);

    return (struct solver_t*)solver;

    alloc_chain_tree_failed : FREE(solver);
    alloc_solver_filed      : return NULL;
}

/* ------------------------------------------------------------------------- */
void
solver_FABRIK_destroy(struct solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    chain_destruct(fabrik->chain_tree);
    FREE(fabrik->chain_tree);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_rebuild_data(struct solver_t* solver)
{
    return rebuild_chain_tree((struct fabrik_t*)solver);
}
/*
static struct vector3_t
solve_chain_forwards(struct chain_t* chain, struct vector3_t target_position)
{
    struct node_t* effector_node = ordered_vector_get_element(&chain->nodes, 0);
    struct vector3_t target = effector_node->effector->target_position;
}*/

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_solve(struct solver_t* solver)
{/*
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    int iteration = solver->max_iterations;
    while(iteration--)
    {
        const struct chain_t* previous_chain = NULL;
        const struct ordered_vector_t* chain_list = &fabrik->chain_list;
        int chain_id = 0;
        while(chain_id != ordered_vector_count(chain_list))
        {
            struct chain_t* chain = ordered_vector_get_element(chain_list, chain_id);
            struct node_t* chain_end = *ordered_vector_get_element(&chain->nodes, 0);
            struct node_t

            *
             * Determine if the current chain is attached to the previous
             * chain's base.
             *
            if(previous_chain != NULL)
            {
                struct node_t** pprevious_chain_beginning =
                    ordered_vector_get_element(&previous_chain->nodes,
                        ordered_vector_count(&previous_chain->nodes) - 1);

                if(chain_end == previous_chain_beginning)
                {

                }
            }

            struct effector_t* effector = chain_end->effector;
            if(effector != NULL)
                *chain->end_position = effector->target_position;
            else if(previous_chain != NULL)
                *chain->end_position = *previous_chain->base_position;
            vector3_set_zero(chain->base_position);

            solve_chain_forwards(chain);
        }
    }*/

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
    struct ordered_vector_t* effector_nodes_list = &solver->effector_nodes_list;
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
int
recursively_build_chain_tree(struct chain_t* chain_current,
                             struct node_t* node_base,
                             struct node_t* node_current,
                             struct bstv_t* involved_nodes)
{
    int marked_children_count;
    struct node_t* child_node_base = node_base;
    struct chain_t* child_chain = chain_current;

    /*
     * If this node is not marked at all, cut off any previous chain and
     * recursive into children. It's possible that there are isolated chains
     * somewhere further down the tree.
     */
    enum node_marking_e marking =
        (enum node_marking_e)(intptr_t)bstv_erase(involved_nodes, node_current->guid);
    /*
     * If this node was marked as the base of a chain then split the chain at
     * this point by updating the base node.
     */
    if(marking == MARK_SPLIT)
        child_node_base = node_current;

    /*
     * If the current node has at least two children marked as sections, then
     * we must also split the chain at this point.
     */
    marked_children_count = 0;
    BSTV_FOR_EACH(&node_current->children, struct node_t, child_guid, child)
        if((enum node_marking_e)(intptr_t)bstv_find(involved_nodes, child_guid) == MARK_SECTION)
            if(++marked_children_count == 2)
                break;
    BSTV_END_EACH

    if(marked_children_count != 1 && node_current != node_base)
    {
        /*
         * Emplace a chain object into the current chain's vector of children
         * and initialise it.
         */
        struct node_t* node;
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

    /*
     * Recurse into children of the current node.
     */
    BSTV_FOR_EACH(&node_current->children, struct node_t, child_guid, child_node)
        if(marking == MARK_NONE)
            child_node_base = child_node;
        if(recursively_build_chain_tree(
                child_chain,
                child_node_base,
                child_node,
                involved_nodes) < 0)
            return -1;
    BSTV_END_EACH

    return 0;
}

static void
dump_chain(struct chain_t* chain, FILE* fp)
{
    int last_idx = ordered_vector_count(&chain->nodes) - 1;
    if(last_idx > 0)
    {
        fprintf(fp, "    %d [shape=record];\n",
            (*(struct node_t**)ordered_vector_get_element(&chain->nodes, 0))->guid);
        fprintf(fp, "    %d [shape=record];\n",
            (*(struct node_t**)ordered_vector_get_element(&chain->nodes, last_idx))->guid);
    }

    while(last_idx-- > 0)
    {
        fprintf(fp, "    %d -- %d [color=\"1.0 0.5 1.0\"];\n",
            (*(struct node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 0))->guid,
            (*(struct node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 1))->guid);
    }

    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        dump_chain(child, fp);
    ORDERED_VECTOR_END_EACH
}

static void
dump_node(struct node_t* node, FILE* fp)
{
    BSTV_FOR_EACH(&node->children, struct node_t, guid, child)
        fprintf(fp, "    %d -- %d;\n", node->guid, guid);
        dump_node(child, fp);
    BSTV_END_EACH
}

static void
dump_to_dot(struct node_t* node, struct chain_t* chain, const char* file_name)
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

static void
count_chains(struct chain_t* chain, int* counter)
{
    ORDERED_VECTOR_FOR_EACH(&chain->children, struct chain_t, child)
        count_chains(child, counter);
    ORDERED_VECTOR_END_EACH
    ++(*counter);
}

/* ------------------------------------------------------------------------- */
static int
rebuild_chain_tree(struct fabrik_t* solver)
{
    struct bstv_t involved_nodes;
    char buffer[20];
    static int file_name_counter = 0;
    struct chain_t* chain_tree = solver->chain_tree;
    struct node_t* root = solver->tree;
    int chain_count = 0;

    /*
     * Build a set of all nodes that are in a direct path with all of the
     * effectors.
     */
    bstv_construct(&involved_nodes);
    if(mark_involved_nodes(solver, &involved_nodes) < 0)
        goto mark_involved_nodes_failed;

    ik_log_message("There are %d involved node(s)", bstv_count(&involved_nodes));

    /* Build a tree of chains */
    chain_clear_free(chain_tree);
    recursively_build_chain_tree(chain_tree, root, root, &involved_nodes);

    /* DEBUG: Save chain tree to DOT */
    sprintf(buffer, "tree%d.dot", file_name_counter++);
    dump_to_dot(root, chain_tree, buffer);

    ik_log_message("There are %d effector(s)",
                   ordered_vector_count(&solver->effector_nodes_list));
    count_chains(chain_tree, &chain_count);
    ik_log_message("There are %d chain(s)", chain_count - 1);

    bstv_clear_free(&involved_nodes);

    return 0;

    mark_involved_nodes_failed : bstv_clear_free(&involved_nodes);
    return -1;
}
