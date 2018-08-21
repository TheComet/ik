#include "ik/bstv.h"
#include "ik/chain.h"
#include "ik/memory.h"
#include "ik/vector.h"
#include "ik/effector.h"
#include "ik/node.h"
#include "ik/log.h"
#include "ik/vec3.h"
#include <assert.h>
#include <stdio.h>

enum node_marking_e
{
    MARK_NONE = 0,
    MARK_BASE,
    MARK_SECTION
};

/* ------------------------------------------------------------------------- */
struct chain_t*
chain_create(void)
{
    struct chain_t* chain = MALLOC(sizeof *chain);
    if (chain == NULL)
    {
        ik_log_fatal("Failed to allocate chain: out of memory");
        return NULL;
    }
    chain_construct(chain);
    return chain;
}

/* ------------------------------------------------------------------------- */
void
chain_destroy(struct chain_t* chain)
{
    chain_destruct(chain);
    FREE(chain);
}

/* ------------------------------------------------------------------------- */
void
chain_construct(struct chain_t* chain)
{
    vector_construct(&chain->nodes, sizeof(struct ik_node_t*));
    vector_construct(&chain->children, sizeof(struct chain_t));
}

/* ------------------------------------------------------------------------- */
void
chain_destruct(struct chain_t* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        chain_destruct(child_chain);
    CHAIN_END_EACH
    vector_clear_free(&chain->children);
    vector_clear_free(&chain->nodes);
}

/* ------------------------------------------------------------------------- */
void
chain_clear_free(struct chain_t* chain)
{
    chain_destruct(chain); /* does the same thing */
}

/* ------------------------------------------------------------------------- */
struct chain_t*
chain_create_child(struct chain_t* chain)
{
    return vector_push_emplace(&chain->children);
}

/* ------------------------------------------------------------------------- */
ikret_t
chain_add_node(struct chain_t* chain, const struct ik_node_t* node)
{
    return vector_push(&chain->nodes, &node);
}

/* ------------------------------------------------------------------------- */
static int
count_chains_recursive(const struct chain_t* chain)
{
    int counter = 1;
    CHAIN_FOR_EACH_CHILD(chain, child)
        counter += count_chains_recursive(child);
    CHAIN_END_EACH
    return counter;
}
int
count_chains(const struct vector_t* chains)
{
    int counter = 0;
    VECTOR_FOR_EACH(chains, struct chain_t, chain)
        counter += count_chains_recursive(chain);
    VECTOR_END_EACH
    return counter;
}

/* ------------------------------------------------------------------------- */
static ikret_t
mark_involved_nodes(struct bstv_t* involved_nodes,
                    const struct vector_t* effector_nodes_list)
{
    /*
     * Traverse the chain of parents starting at each effector node and ending
     * at the sub-base node of the tree and mark every node on the way. Each
     * effector specifies a maximum chain length, which means it's possible
     * that we won't hit the base node.
     */
    VECTOR_FOR_EACH(effector_nodes_list, struct ik_node_t*, p_effector_node)

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
         *
         * Additionally, there is a special constraint (IK_CONSTRAINT_STIFF)
         * that restricts all rotations of a node. If this constraint is
         * imposed on a particular node, mark it differently as well so the
         * surrounding nodes can be combined into a single bone properly later.
         *
         * NOTE: The node->constraint field specifies constraints for
         * the *parent* node, not for the current node. However, we will be
         * marking the *current* node, not the parent node.
         */
        for (; node != NULL; node = node->parent)
        {
            enum node_marking_e* current_marking;
            enum node_marking_e marking = MARK_SECTION;
            if (chain_length_counter == 0)
                marking = MARK_BASE;

            current_marking = (enum node_marking_e*)bstv_find_ptr(involved_nodes, node->guid);
            if (current_marking == NULL)
            {
                if (bstv_insert(involved_nodes, node->guid, (void*)(intptr_t)marking) < 0)
                {
                    ik_log_fatal("Ran out of memory while marking involved nodes");
                    return IK_RAN_OUT_OF_MEMORY;
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
    VECTOR_END_EACH

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static ikret_t
recursively_build_chain_tree(struct vector_t* chain_list,
                             struct chain_t* chain_current,
                             const struct ik_node_t* node_base,
                             const struct ik_node_t* node_current,
                             struct bstv_t* involved_nodes)
{
    int marked_children_count;
    const struct ik_node_t* child_node_base = node_base;
    struct chain_t* child_chain = chain_current;

    /* can remove the mark from the set to speed up future checks */
    enum node_marking_e marking =
        (enum node_marking_e)(intptr_t)bstv_erase(involved_nodes, node_current->guid);

    switch(marking)
    {
        /*
         * If this node was marked as the base of a chain then split the chain
         * at this point by moving the pointer to the base node down the tree
         * to the current node and set the current chain to NULL so a new
         * island is created (this is necessary because all children of this
         * node are necessarily part of an isolated tree).
         */
        case MARK_BASE:
            child_node_base = node_current;
            chain_current = NULL;
            break;
        /*
         * If this node is not marked at all, cut off any previous chain but
         * continue (fall through) as if a section was marked. It's possible
         * that there are isolated chains somewhere further down the tree.
         */
        case MARK_NONE:
            node_base = node_current;
            /* falling through on purpose */

        case MARK_SECTION:
            /*
             * If the current node has at least two children marked as sections
             * or if (the current node is an effector node, but only if (the base
             * node is not equal to this node (that is, we need to avoid chains
             * that would have less than 2 nodes), then we must also split the
             * chain at this point.
             */
            marked_children_count = 0;
            NODE_FOR_EACH(node_current, child_guid, child)
                if ((enum node_marking_e)(intptr_t)bstv_find(involved_nodes, child_guid) == MARK_SECTION)
                    if (++marked_children_count == 2)
                        break;
            NODE_END_EACH
            if ((marked_children_count == 2 || node_current->effector != NULL) && node_current != node_base)
            {
                const struct ik_node_t* node;

                if (chain_current == NULL) /* First chain in the tree? */
                {
                    /* Insert and initialize a base chain in the list. */
                    child_chain = vector_push_emplace(chain_list);
                    if (child_chain == NULL)
                    {
                        ik_log_fatal("Failed to create base chain: Ran out of memory");
                        return IK_RAN_OUT_OF_MEMORY;
                    }
                    chain_construct(child_chain);
                }
                else /* This is not the first chain in the tree */
                {
                    /*
                     * Create a new child chain in the current chain and
                     * initialize it.
                     */
                    child_chain = chain_create_child(chain_current);
                    if (child_chain == NULL)
                    {
                        ik_log_fatal("Failed to create child chain: Ran out of memory");
                        return IK_RAN_OUT_OF_MEMORY;
                    }
                    chain_construct(child_chain);
                }

                /*
                 * Add pointers to all nodes that are part of this chain into
                 * the chain's list, starting with the end node.
                 */
                for (node = node_current; node != node_base; node = node->parent)
                    if (chain_add_node(child_chain, node) != 0)
                    {
                        ik_log_fatal("Failed to insert node into chain: Ran out of memory");
                        return IK_RAN_OUT_OF_MEMORY;
                    }
                if (chain_add_node(child_chain, node_base) != 0)
                {
                    ik_log_fatal("Failed to insert node into chain: Ran out of memory");
                    return IK_RAN_OUT_OF_MEMORY;
                }

                /*
                 * Update the base node to be this node so deeper chains are
                 * built back to this node
                 */
                child_node_base = node_current;
            }
            break;
    }

    /* Recurse into children of the current node. */
    NODE_FOR_EACH(node_current, child_guid, child_node)
        ikret_t result;
        if ((result = recursively_build_chain_tree(
                chain_list,
                child_chain,
                child_node_base,
                child_node,
                involved_nodes)) != IK_OK)
            return result;
    NODE_END_EACH

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
ikret_t
chain_tree_rebuild(struct vector_t* chain_list,
                   const struct ik_node_t* base_node,
                   const struct vector_t* effector_nodes_list)
{
    ikret_t result;
    struct bstv_t involved_nodes;
    int involved_nodes_count;
#ifdef IK_DOT_OUTPUT
    char buffer[20];
    static int file_name_counter = 0;
#endif

    /* Clear all existing chain trees */
    VECTOR_FOR_EACH(chain_list, struct chain_t, chain)
        chain_destruct(chain);
    VECTOR_END_EACH
    vector_clear_free(chain_list);

    /*
     * Build a set of all nodes that are in a direct path with all of the
     * effectors.
     */
    bstv_construct(&involved_nodes);
    if ((result = mark_involved_nodes(&involved_nodes, effector_nodes_list)) != IK_OK)
        goto mark_involved_nodes_failed;
    involved_nodes_count = bstv_count(&involved_nodes);

    recursively_build_chain_tree(chain_list, NULL, base_node, base_node, &involved_nodes);

    /* DEBUG: Save chain tree to DOT */
#ifdef IK_DOT_OUTPUT
    sprintf(buffer, "tree%d.dot", file_name_counter++);
    dump_to_dot(base_node, chains, buffer);
#endif

    ik_log_info("There are %d effector(s) involving %d node(s). %d chain(s) were created",
                   vector_count(effector_nodes_list),
                   involved_nodes_count,
                   count_chains(chain_list));

    bstv_clear_free(&involved_nodes);

    return result;

    mark_involved_nodes_failed : bstv_clear_free(&involved_nodes);
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static void
calculate_segment_lengths_in_island(struct chain_t* chain)
{
    /*
     * The nodes are in local space, so the segment length is just the length
     * of the node->position vector.
     *
     * The segment length of a node refers to the distance to its parent rather
     * than to the distance to its child. Thus, the base node of a chain does
     * not need to be iterated.
     */
    int last_idx = chain_length(chain) - 1;
    while (last_idx-- > 0)
    {
        struct ik_node_t* node = chain_get_node(chain, last_idx);

        node->dist_to_parent = ik_vec3_length(node->position.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        calculate_segment_lengths_in_island(child);
    CHAIN_END_EACH
}
void
update_distances(const struct vector_t* chains)
{
    /* TODO: Implement again, take into consideration bone skipping */
    VECTOR_FOR_EACH(chains, struct chain_t, chain)
        calculate_segment_lengths_in_island(chain);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
#ifdef IK_DOT_OUTPUT
static void
dump_chain(const chain_t* chain, FILE* fp)
{
    int last_idx = chain_length(chain) - 1;
    if (last_idx > 0)
    {
        fprintf(fp, "    %d [shape=record];\n", chain_get_node(chain, 0)->guid);
        fprintf(fp, "    %d [shape=record];\n", chain_get_base_node(chain)->guid);
    }

    while (last_idx-- > 0)
    {
        fprintf(fp, "    %d -- %d [color=\"1.0 0.5 1.0\"];\n",
            chain_get_node(chain, last_idx + 0)->guid,
            chain_get_node(chain, last_idx + 1)->guid);
    }

    VECTOR_FOR_EACH(&chain->children, chain_t, child)
        dump_chain(child, fp);
    VECTOR_END_EACH
}
static void
dump_node(const ik_node_t* node, FILE* fp)
{
    if (node->effector != NULL)
        fprintf(fp, "    %d [color=\"0.6 0.5 1.0\"];\n", node->guid);
    NODE_FOR_EACH(node, guid, child)
        fprintf(fp, "    %d -- %d;\n", node->guid, guid);
        dump_node(child, fp);
    NODE_END_EACH
}
void
dump_to_dot(const ik_node_t* node, const vector_t* chains, const char* file_name)
{
    FILE* fp = fopen(file_name, "w");
    if (fp == NULL)
        return;

    fprintf(fp, "graph chain_tree {\n");
    dump_node(node, fp);
    VECTOR_FOR_EACH(chains, chain_t, chain)
        dump_chain(chain, fp);
    VECTOR_END_EACH
    fprintf(fp, "}\n");

    fclose(fp);
}
#endif
