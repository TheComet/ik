#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node_tree_flattened.h"
#include "ik/node.h"
#include "ik/node_data.h"
#include "ik/vector.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>

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
mark_nodes(struct btree_t* marked, struct vector_t* effector_nodes)
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
            enum mark_e* current_marking = (enum mark_e*)btree_find_ptr(marked, ik_node_get_uid(node));
            if (current_marking != NULL)
            {
                *current_marking = MARK_SECTION;
            }
            else
            {
                ikret_t status;
                if ((status = btree_insert(marked, ik_node_get_uid(node),
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
static ikret_t
split_into_subtrees(struct vector_t* tree_list,
                    const struct ik_node_t* node,
                    const struct btree_t* marked_nodes)
{
    /*
     * If this node has the "base" marking, add it to the list of root nodes,
     * as it is a root node of one of the isolated trees.
     */
    enum mark_e* marking = (enum mark_e*)btree_find_ptr(marked_nodes, ik_node_get_uid(node));
    if (marking != NULL && *marking == MARK_BASE)
    {
        ikret_t status;
        if ((status = vector_push(tree_list, &node)) != IK_OK)
            return status;
    }

    /* Recurse into children */
    NODE_FOR_EACH(node, uid, child)
        ikret_t status;
        if ((status = split_into_subtrees(tree_list, child, marked_nodes)) != IK_OK)
            return status;
    NODE_END_EACH

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static uintptr_t
count_marked_nodes_in_subtree(const struct ik_node_t* node,
                              const struct btree_t* marked_nodes,
                              uintptr_t counter)
{
    NODE_FOR_EACH(node, uid, child)
        enum mark_e* marking =
            (enum mark_e*)btree_find_ptr(marked_nodes, ik_node_get_uid(node));
        if (marking != NULL && *marking == MARK_SECTION)
            counter += count_marked_nodes_in_subtree(child, marked_nodes, 0);
    NODE_END_EACH

    return counter;
}
#define count_marked_nodes_in_subtree(node, marked_nodes) \
    count_marked_nodes_in_subtree(node, marked_nodes, 1)

/* ------------------------------------------------------------------------- */
static ikret_t
process_subtree(struct vector_t* flattened_trees,
                const struct ik_node_t* root,
                const struct btree_t* marked_nodes)
{
    ikret_t status;

    struct ik_ntf_t* ntf = vector_emplace(flattened_trees);
    if (ntf == NULL)
    {
        ik_log_fatal("Failed to emplace new flattened tree structure: Ran out of memory");
        FAIL(IK_ERR_OUT_OF_MEMORY, emplace_failed);
    }

    ik_ntf_construct(ntf);
    uintptr_t node_count = count_marked_nodes_in_subtree(root, marked_nodes);
    ntf->nodes = MALLOC(sizeof(struct ik_node_t) * node_count);
    if (ntf->nodes == NULL)
    {
        ik_log_fatal("Failed to allocate flattened node array: Ran out of memory");
        FAIL(IK_ERR_OUT_OF_MEMORY, alloc_nodes_array_failed);
    }

    return IK_OK;

    alloc_nodes_array_failed : vector_pop(flattened_trees);
    emplace_failed           : return status;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_ntf_create(struct ik_ntf_t** ntf)
{
    assert(ntf);
    *ntf = MALLOC(sizeof **ntf);
    if (*ntf == NULL)
    {
        ik_log_fatal("Failed to allocate NTF: Ran out of memory");
        return IK_ERR_OUT_OF_MEMORY;
    }

    ik_ntf_construct(*ntf);

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_ntf_construct(struct ik_ntf_t* ntf)
{
    memset(ntf, 0, sizeof *ntf);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_ntf_from_nodes(struct ik_ntf_t* ntf, const struct ik_node_t* root)
{
    ikret_t status;
    struct vector_t effector_nodes;
    struct vector_t tree_list;
    struct btree_t marked_nodes;
    struct vector_t* flattened_trees;

    /* Create list of all nodes that have effectors attached */
    vector_construct(&effector_nodes, sizeof(struct ik_node_t*));
    if ((status = find_all_effector_nodes(&effector_nodes, root)) != IK_OK)
        goto find_effectors_failed;
    if (vector_count(&effector_nodes) == 0)
    {
        ik_log_warning("No effectors were found in the tree. Not building flattened tree structure.");
        FAIL(IK_OK, find_effectors_failed);
    }

    /* Mark all nodes that the solver can reach */
    btree_construct(&marked_nodes);
    if ((status = mark_nodes(&marked_nodes, &effector_nodes)) != IK_OK)
        goto mark_nodes_failed;

    /*
     * It's possible that chain length limits end up isolating parts of the
     * tree, splitting it into a list of "sub-trees" which must be solved
     * "in-order" (LNR).
     */
    vector_construct(&tree_list, sizeof(struct ik_node_t*));
    if ((status = split_into_subtrees(&tree_list, root, &marked_nodes)) != IK_OK)
        goto split_into_islands_failed;

    vector_create(&flattened_trees, sizeof(struct ik_ntf_t));
    VECTOR_FOR_EACH(&tree_list, struct ik_node_t*, node)
        if ((status = process_subtree(flattened_trees, *node, &marked_nodes)) != IK_OK)
            goto process_subtrees_failed;
    VECTOR_END_EACH

    vector_clear_free(&tree_list);
    btree_clear_free(&marked_nodes);
    vector_clear_free(&effector_nodes);

    return IK_OK;

    process_subtrees_failed   :
    split_into_islands_failed : vector_clear_free(&tree_list);
    mark_nodes_failed         : btree_clear_free(&marked_nodes);
    find_effectors_failed     : vector_clear_free(&effector_nodes);
    return status;
}
