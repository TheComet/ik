#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node_tree_flattened.h"
#include "ik/node.h"
#include "ik/node_data.h"
#include "ik/vector.h"
#include <stddef.h>
#include <assert.h>

#define FAIL(errcode, label) do { status = errcode; goto label; } while(0)

enum mark_e
{
    MARK_NONE,
    MARK_SECTION,
    MARK_BASE,
    MARK_STIFF
};

/* ------------------------------------------------------------------------- */
static ikret_t
find_all_effector_nodes(struct vector_t* result, const struct ik_node_t* node)
{
    ikret_t status;
    NODE_FOR_EACH(node, user_data, child)
        if ((status = find_all_effector_nodes(result, node)) != IK_OK)
            return status;
    NODE_END_EACH

    if (ik_node_get_effector(node) != NULL)
        if ((status = vector_push(result, &node)) != IK_OK)
            return status;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static ikret_t
mark_nodes(struct bstv_t* marked, struct vector_t* effector_nodes)
{
    /*
     * Traverse the chain of nodes starting at each effector node and ending
     * at the specified chain length of the effector, mark every node on the
     * way.
     */
    VECTOR_FOR_EACH(effector_nodes, struct ik_node_t*, p_effector_node)

        int chain_length_counter;
        struct ik_node_t* node                = *p_effector_node;
        const struct ik_effector_t* effector  = ik_node_get_effector(node);

        /*
         * Set up chain length counter. If the chain length is 0 then it is
         * infinitely long. Set the counter to -1 in this case to skip the
         * escape condition.
         */
        assert(effector != NULL);
        chain_length_counter = effector->chain_length == 0 ?
                -1 : (int)effector->chain_length;

        /*
         * Walk up chain (starting at effector node and ending if we run out of
         * nodes, or the chain length counter reaches 0). Mark every node in
         * the chain as MARK_SECTION. If we get to the last node in the chain,
         * mark it as MARK_BASE only if the node is unmarked. This means that
         * nodes marked as MARK_BASE will be overwritten with MARK_SECTION if
         * necessary.
         */
        for (; node != NULL; node = node->parent)
        {
            enum mark_e* current_marking = (enum mark_e*)bstv_find_ptr(marked, ik_node_get_guid(node));
            if (current_marking != NULL)
            {
                *current_marking = MARK_SECTION;
            }
            else
            {
                ikret_t status;
                if ((status = bstv_insert(marked, ik_node_get_guid(node),
                    (void*)(intptr_t)MARK_BASE) < 0) != IK_OK)
                {
                    ik_log_fatal("Ran out of memory while marking involved nodes");
                    return status;
                }
            }

            if (chain_length_counter-- == 0)
                break;
        }
    VECTOR_END_EACH

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
IK_PRIVATE_API ikret_t
ik_node_tree_flattened_create(struct ik_node_tree_flattened_t** ntf,
                              struct ik_node_t* root)
{
    ikret_t status;
    struct vector_t effector_nodes;
    struct bstv_t node_markings;

    vector_construct(&effector_nodes, sizeof(struct ik_node_t*));

    if ((status = find_all_effector_nodes(&effector_nodes, root)) != IK_OK)
        goto find_effectors_failed;
    if (vector_count(&effector_nodes) == 0)
    {
        ik_log_warning("No effectors were found in the tree. Not building flattened tree structure.");
        FAIL(IK_OK, find_effectors_failed);
    }

    bstv_construct(&node_markings);
    if ((status = mark_nodes(&node_markings, &effector_nodes)) != IK_OK)
        goto mark_nodes_failed;



    bstv_clear_free(&node_markings);
    vector_clear_free(&effector_nodes);

    return IK_OK;

    mark_nodes_failed     : bstv_clear_free(&node_markings);
    find_effectors_failed : vector_clear_free(&effector_nodes);
    return status;
}

/* ------------------------------------------------------------------------- */
