#include "ik/bst_vector.h"
#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/ordered_vector.h"
#include "ik/solver.h"
#include <assert.h>
#include <stdio.h>

enum node_marking_e
{
    MARK_NONE = 0,
    MARK_SPLIT,
    MARK_SECTION
};

/* ------------------------------------------------------------------------- */
ik_chain_t*
chain_create(void)
{
    ik_chain_t* chain = (ik_chain_t*)MALLOC(sizeof *chain);
    if (chain == NULL)
    {
        ik_log_message("Failed to allocate chain: out of memory");
        return NULL;
    }
    chain_construct(chain);
    return chain;
}

/* ------------------------------------------------------------------------- */
void
chain_destroy(ik_chain_t* chain)
{
    chain_destruct(chain);
    FREE(chain);
}

/* ------------------------------------------------------------------------- */
void
chain_construct(ik_chain_t* chain)
{
    ordered_vector_construct(&chain->nodes, sizeof(ik_node_t*));
    ordered_vector_construct(&chain->children, sizeof(ik_chain_t));
}

/* ------------------------------------------------------------------------- */
void
chain_clear_free(ik_chain_t* chain)
{
    chain_destruct(chain); /* does the same thing as de*/
}

/* ------------------------------------------------------------------------- */
void
chain_destruct(ik_chain_t* chain)
{
    ORDERED_VECTOR_FOR_EACH(&chain->children, ik_chain_t, child_chain)
        chain_destruct(child_chain);
    ORDERED_VECTOR_END_EACH
    ordered_vector_clear_free(&chain->children);
    ordered_vector_clear_free(&chain->nodes);
}

/* ------------------------------------------------------------------------- */
static int
count_chains_recursive(ik_chain_t* chain)
{
    int counter = 1;
    ORDERED_VECTOR_FOR_EACH(&chain->children, ik_chain_t, child)
        counter += count_chains_recursive(child);
    ORDERED_VECTOR_END_EACH
    return counter;
}
int
count_chains_exclude_root(ik_chain_t* chain)
{
    int counter = 1;
    ORDERED_VECTOR_FOR_EACH(&chain->children, ik_chain_t, child)
        counter += count_chains_recursive(child);
    ORDERED_VECTOR_END_EACH
    return counter - 1; /* exclude root chain */
}

/* ------------------------------------------------------------------------- */
#if IK_DOT_OUTPUT == ON
static void
dump_chain(ik_chain_t* chain, FILE* fp)
{
    int last_idx = ordered_vector_count(&chain->nodes) - 1;
    if (last_idx > 0)
    {
        fprintf(fp, "    %d [shape=record];\n",
            (*(ik_node_t**)ordered_vector_get_element(&chain->nodes, 0))->guid);
        fprintf(fp, "    %d [shape=record];\n",
            (*(ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx))->guid);
    }

    while (last_idx-- > 0)
    {
        fprintf(fp, "    %d -- %d [color=\"1.0 0.5 1.0\"];\n",
            (*(ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 0))->guid,
            (*(ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 1))->guid);
    }

    ORDERED_VECTOR_FOR_EACH(&chain->children, ik_chain_t, child)
        dump_chain(child, fp);
    ORDERED_VECTOR_END_EACH
}
static void
dump_node(ik_node_t* node, FILE* fp)
{
    if (node->effector != NULL)
        fprintf(fp, "    %d [color=\"0.6 0.5 1.0\"];\n", node->guid);
    BSTV_FOR_EACH(&node->children, ik_node_t, guid, child)
        fprintf(fp, "    %d -- %d;\n", node->guid, guid);
        dump_node(child, fp);
    BSTV_END_EACH
}
void
dump_to_dot(ik_node_t* node, ik_chain_t* chain, const char* file_name)
{
    FILE* fp = fopen(file_name, "w");
    if (fp == NULL)
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
mark_involved_nodes(ik_solver_t* solver, bstv_t* involved_nodes)
{
    /*
     * Traverse the chain of parents starting at each effector node and ending
     * at the root node of the tree and mark every node on the way. Each
     * effector specifies a maximum chain length, which means it's possible
     * that we won't hit the root node.
     */
    ordered_vector_t* effector_nodes_list = &solver->effector_nodes_list;
    ORDERED_VECTOR_FOR_EACH(effector_nodes_list, ik_node_t*, p_effector_node)

        /*
         * Set up chain length counter. If the chain length is 0 then it is
         * infinitely long. Set the counter to -1 in this case to skip the
         * escape condition.
         */
        int chain_length_counter;
        ik_node_t* node = *p_effector_node;
        assert(node->effector != NULL);
        chain_length_counter = node->effector->chain_length == 0 ? -1 : (int)node->effector->chain_length;

        /*
         * Mark nodes that are at the base of the chain differently, so the
         * chains can be split correctly later. Section markings will overwrite
         * break markings.
         */
        for (; node != NULL; node = node->parent)
        {
            enum node_marking_e* current_marking;
            enum node_marking_e marking = MARK_SECTION;
            if (chain_length_counter == 0)
                marking = MARK_SPLIT;

            current_marking = (enum node_marking_e*)bstv_find_ptr(involved_nodes, node->guid);
            if (current_marking == NULL)
            {
                if (bstv_insert(involved_nodes, node->guid, (void*)(intptr_t)marking) < 0)
                {
                    ik_log_message("Ran out of memory while marking involved nodes");
                    return -1;
                }
            }
            else
            {
                if (chain_length_counter != 0)
                    *current_marking = marking;
            }

            if (chain_length_counter-- == 0)
                break;
        }
    ORDERED_VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
recursively_build_chain_tree(ik_chain_t* chain_current,
                             ik_node_t* node_base,
                             ik_node_t* node_current,
                             bstv_t* involved_nodes)
{
    int marked_children_count;
    ik_node_t* child_node_base = node_base;
    ik_chain_t* child_chain = chain_current;

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
         * continue (fall through) as if (a section was marked. It's possible
         * that there are isolated chains somewhere further down the tree.
         */
        case MARK_NONE:
            node_base = node_current;

        case MARK_SECTION:
            /*
             * If the current node has at least two children marked as sections
             * or if (the current node is an effector node, but only if (the base
             * node is not equal to this node (that is, we need to avoid chains
             * that would have less than 2 nodes), then we must also split the
             * chain at this point.
             */
            marked_children_count = 0;
            BSTV_FOR_EACH(&node_current->children, ik_node_t, child_guid, child)
                if ((enum node_marking_e)(intptr_t)bstv_find(involved_nodes, child_guid) == MARK_SECTION)
                    if (++marked_children_count == 2)
                        break;
            BSTV_END_EACH
            if ((marked_children_count == 2 || node_current->effector != NULL) && node_current != node_base)
            {
                /*
                 * Emplace a chain object into the current chain's vector of children
                 * and initialise it.
                 */
                ik_node_t* node;
                child_chain = ordered_vector_push_emplace(&chain_current->children);
                if (child_chain == NULL)
                    return -1;
                chain_construct(child_chain);

                /*
                 * Add points to all nodes that are part of this chain into the chain's
                 * list, starting with the end node.
                 */
                for (node = node_current; node != node_base; node = node->parent)
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
    BSTV_FOR_EACH(&node_current->children, ik_node_t, child_guid, child_node)
        if (recursively_build_chain_tree(
                child_chain,
                child_node_base,
                child_node,
                involved_nodes) < 0)
            return -1;
    BSTV_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
rebuild_chain_tree(ik_solver_t* solver)
{
    bstv_t involved_nodes;
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
    if (mark_involved_nodes(solver, &involved_nodes) < 0)
        goto mark_involved_nodes_failed;
    involved_nodes_count = bstv_count(&involved_nodes);

    /*
     * The user can choose to set the root node as a chain terminator (default)
     * or choose to exclude the root node, in which case each immediate child
     * of the tree is a chain terminator. In this case we need to build the
     * chain tree for each child individually.
     */
    chain_clear_free(solver->chain_tree);
    if (solver->flags & SOLVER_EXCLUDE_ROOT)
    {
        BSTV_FOR_EACH(&solver->tree->children, ik_node_t, guid, child)
            recursively_build_chain_tree(solver->chain_tree, child, child, &involved_nodes);
        BSTV_END_EACH
    }
    else
    {
        recursively_build_chain_tree(solver->chain_tree, solver->tree, solver->tree, &involved_nodes);
    }

    /* Pre-compute offsets for each node in the chain tree in relation to their
     * parents */
    calculate_segment_lengths(solver->chain_tree);

    /* DEBUG: Save chain tree to DOT */
#if IK_DOT_OUTPUT == ON
    sprintf(buffer, "tree%d.dot", file_name_counter++);
    dump_to_dot(solver->tree, solver->chain_tree, buffer);
#endif

    ik_log_message("There are %d effector(s) involving %d node(s). %d chain(s) were created",
                   ordered_vector_count(&solver->effector_nodes_list),
                   involved_nodes_count,
                   count_chains_exclude_root(solver->chain_tree) - 1); /* don't count root chain which always exists */

    bstv_clear_free(&involved_nodes);

    return 0;

    mark_involved_nodes_failed : bstv_clear_free(&involved_nodes);
    return -1;
}

/* ------------------------------------------------------------------------- */
void
calculate_segment_lengths(ik_chain_t* chain)
{
    int last_idx = ordered_vector_count(&chain->nodes) - 1;
    while (last_idx-- > 0)
    {
        ik_node_t* child_node =
            *(ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 0);
        ik_node_t* parent_node =
            *(ik_node_t**)ordered_vector_get_element(&chain->nodes, last_idx + 1);

        vec3_t diff = child_node->initial_position;
        vec3_sub_vec3(diff.f, parent_node->initial_position.f);
        child_node->segment_length = vec3_length(diff.f);
    }

    ORDERED_VECTOR_FOR_EACH(&chain->children, ik_chain_t, child)
        calculate_segment_lengths(child);
    ORDERED_VECTOR_END_EACH
}
