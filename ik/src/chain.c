#include "ik/bst_vector.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/vector.h"
#include "ik/solver.h"
#include <assert.h>
#include <stdio.h>

enum node_marking_e
{
    MARK_NONE = 0,
    MARK_BASE,
    MARK_SECTION
};

/* ------------------------------------------------------------------------- */
void
base_chain_construct(base_chain_t* base_chain)
{
    chain_construct((chain_t*)base_chain);
    vector_construct(&base_chain->data.base_chain.connecting_nodes, sizeof(ik_node_t*));
}

/* ------------------------------------------------------------------------- */
void
base_chain_destruct(base_chain_t* base_chain)
{
    vector_clear_free(&base_chain->data.base_chain.connecting_nodes);
    chain_destruct((chain_t*)base_chain);
}

/* ------------------------------------------------------------------------- */
chain_t*
chain_create(void)
{
    chain_t* chain = (chain_t*)MALLOC(sizeof *chain);
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
chain_destroy(chain_t* chain)
{
    chain_destruct(chain);
    FREE(chain);
}

/* ------------------------------------------------------------------------- */
void
chain_construct(chain_t* chain)
{
    vector_construct(&chain->data.chain.nodes, sizeof(ik_node_t*));
    vector_construct(&chain->data.chain.children, sizeof(chain_t));
}

/* ------------------------------------------------------------------------- */
void
chain_destruct(chain_t* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        chain_destruct(child_chain);
    CHAIN_END_EACH
    vector_clear_free(&chain->data.chain.children);
    vector_clear_free(&chain->data.chain.nodes);
}

/* ------------------------------------------------------------------------- */
void
chain_clear_free(chain_t* chain)
{
    chain_destruct(chain); /* does the same thing as de*/
}

/* ------------------------------------------------------------------------- */
chain_t*
chain_create_child(chain_t* chain)
{
    return vector_push_emplace(&chain->data.chain.children);
}

int
chain_add_node(chain_t* chain, const ik_node_t* node)
{
    return vector_push(&chain->data.chain.nodes, &node);
}

/* ------------------------------------------------------------------------- */
static int
count_chains_recursive(const chain_t* chain)
{
    int counter = 1;
    CHAIN_FOR_EACH_CHILD(chain, child)
        counter += count_chains_recursive(child);
    CHAIN_END_EACH
    return counter;
}
int
count_chains(const vector_t* chains)
{
    int counter = 0;
    VECTOR_FOR_EACH(chains, chain_t, chain)
        counter += count_chains_recursive(chain);
    VECTOR_END_EACH
    return counter;
}

/* ------------------------------------------------------------------------- */
static int
mark_involved_nodes(bstv_t* involved_nodes,
                    const vector_t* effector_nodes_list)
{
    /*
     * Traverse the chain of parents starting at each effector node and ending
     * at the sub-base node of the tree and mark every node on the way. Each
     * effector specifies a maximum chain length, which means it's possible
     * that we won't hit the base node.
     */
    VECTOR_FOR_EACH(effector_nodes_list, ik_node_t*, p_effector_node)

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
    VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
recursively_build_chain_tree(vector_t* base_chain_list,
                             chain_t* chain_current,
                             const ik_node_t* node_base,
                             const ik_node_t* node_current,
                             bstv_t* involved_nodes)
{
    int marked_children_count;
    const ik_node_t* child_node_base = node_base;
    chain_t* child_chain = chain_current;

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
                const ik_node_t* node;

                if (chain_current == NULL) /* First chain in the tree? */
                {
                    /* Insert and initialise a base chain in the list. */
                    child_chain = (chain_t*)vector_push_emplace(base_chain_list);
                    if (child_chain == NULL)
                    {
                        ik_log_message("Failed to create base chain: Ran out of memory");
                        return -1;
                    }
                    chain_construct(child_chain);
                }
                else /* This is not the first chain in the tree */
                {
                    /*
                     * Create a new child chain in the current chain and
                     * initialise it.
                     */
                    child_chain = chain_create_child(chain_current);
                    if (child_chain == NULL)
                    {
                        ik_log_message("Failed to create child chain: Ran out of memory");
                        return -1;
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
                        ik_log_message("Failed to insert node into chain: Ran out of memory");
                        return -1;
                    }
                if (chain_add_node(child_chain, node_base) != 0)
                {
                    ik_log_message("Failed to insert node into chain: Ran out of memory");
                    return -1;
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
        if (recursively_build_chain_tree(
                base_chain_list,
                child_chain,
                child_node_base,
                child_node,
                involved_nodes) < 0)
            return -1;
    NODE_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
chain_tree_rebuild(vector_t* base_chain_list,
                   const ik_node_t* base_node,
                   const vector_t* effector_nodes_list)
{
    bstv_t involved_nodes;
    int involved_nodes_count;
#ifdef IK_DOT_OUTPUT
    char buffer[20];
    static int file_name_counter = 0;
#endif

    /* Clear all existing chain trees */
    VECTOR_FOR_EACH(base_chain_list, chain_t, chain)
        chain_destruct(chain);
    VECTOR_END_EACH
    vector_clear_free(base_chain_list);

    /*
     * Build a set of all nodes that are in a direct path with all of the
     * effectors.
     */
    bstv_construct(&involved_nodes);
    if (mark_involved_nodes(&involved_nodes, effector_nodes_list) < 0)
        goto mark_involved_nodes_failed;
    involved_nodes_count = bstv_count(&involved_nodes);

    recursively_build_chain_tree(base_chain_list, NULL, base_node, base_node, &involved_nodes);

    /* DEBUG: Save chain tree to DOT */
#ifdef IK_DOT_OUTPUT
    sprintf(buffer, "tree%d.dot", file_name_counter++);
    dump_to_dot(base_node, chains, buffer);
#endif

    ik_log_message("There are %d effector(s) involving %d node(s). %d chain(s) were created",
                   vector_count(effector_nodes_list),
                   involved_nodes_count,
                   count_chains(base_chain_list));

    bstv_clear_free(&involved_nodes);

    return 0;

    mark_involved_nodes_failed : bstv_clear_free(&involved_nodes);
    return -1;
}

/* ------------------------------------------------------------------------- */
static void
calculate_segment_lengths_in_island(chain_t* chain)
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
        ik_node_t* node = chain_get_node(chain, last_idx);

        node->segment_length = vec3_length(node->position.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        calculate_segment_lengths_in_island(child);
    CHAIN_END_EACH
}
void
calculate_segment_lengths(const vector_t* chains)
{
    /* TODO: Implement again, take into consideration bone skipping */
    VECTOR_FOR_EACH(chains, chain_t, chain)
        calculate_segment_lengths_in_island(chain);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
calculate_global_rotations_of_children(const chain_t* chain)
{
    int average_count;
    quat_t average_rotation = {{0, 0, 0, 0}};

    /* Recurse into children chains */
    average_count = 0;
    CHAIN_FOR_EACH_CHILD(chain, child)
        quat_t rotation;
        calculate_global_rotations(child);

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
calculate_delta_rotation_of_each_segment(const chain_t* chain)
{
    int node_idx;

    /*
     * Calculate all of the delta angles of the joints. The resulting delta (!)
     * angles will be written to node->rotation
     */
    node_idx = chain_length(chain) - 1;
    while (node_idx-- > 0)
    {
        ik_node_t* child_node  = chain_get_node(chain, node_idx + 0);
        ik_node_t* parent_node = chain_get_node(chain, node_idx + 1);

        /* calculate vectors for original and solved segments */
        vec3_t segment_original = child_node->original_position;
        vec3_t segment_solved   = child_node->position;
        vec3_sub_vec3(segment_original.f, parent_node->original_position.f);
        vec3_sub_vec3(segment_solved.f, parent_node->position.f);

        vec3_angle(parent_node->rotation.f, segment_original.f, segment_solved.f);
    }
}

/* ------------------------------------------------------------------------- */
void
calculate_global_rotations(const chain_t* chain)
{
    int node_idx;

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

    calculate_global_rotations_of_children(chain);
    calculate_delta_rotation_of_each_segment(chain);

    /*
     * At this point, all nodes have calculated their delta angles *except* for
     * the end effector nodes, which remain untouched. It makes sense to copy
     * the delta rotation of the parent node into the effector node by default.
     */
    node_idx = chain_length(chain);
    if (node_idx > 1)
    {
        ik_node_t* effector_node  = chain_get_node(chain, 0);
        ik_node_t* parent_node    = chain_get_node(chain, 1);
        effector_node->rotation.q = parent_node->rotation.q;
    }

    /*
     * Finally, apply initial global rotations to calculated delta rotations to
     * obtain the solved global rotations.
     */
    CHAIN_FOR_EACH_NODE(chain, node)
        quat_mul_quat(node->rotation.f, node->original_rotation.f);
    CHAIN_END_EACH
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
