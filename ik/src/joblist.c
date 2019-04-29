#include "ik/joblist.h"

#include "cstructures/memory.h"
#include "cstructures/vector.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

enum mark_e
{
    MARK_SECTION,
    MARK_SPLIT
};

/* ------------------------------------------------------------------------- */
static uint32_t
count_children_marked(const struct ik_node_t* node,
                      const struct btree_t* marked_nodes)
{
    uint32_t count = 0;
    NODE_FOR_EACH(node, uid, child)
        if (btree_find(marked_nodes, IK_NODE_USER_DATA(node)))
            count++;
    NODE_END_EACH

    return count;
}

/* ------------------------------------------------------------------------- */
static ikret_t
find_all_effector_nodes(struct vector_t* result, const struct ik_node_t* node)
{
    ikret_t status;
    NODE_FOR_EACH(node, user_data, child)
        if ((status = find_all_effector_nodes(result, child)) != IK_OK)
            return status;
    NODE_END_EACH

    if (IK_NODE_EFFECTOR(node) != NULL)
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
        const struct ik_effector_t* effector  = IK_NODE_EFFECTOR(node);

        /*
         * Set up chain length counter. If the chain length is 0 then it is
         * infinitely long. Set the counter to -1 in this case to skip the
         * escape condition. A chain length of 1 means it affects 1 *bone*, but
         * we are traversing nodes -- 2 nodes form one bone. Therefore, we must
         * add 1 to the chain length.
         */
        assert(effector != NULL);
        chain_length_counter = effector->chain_length == 0 ?
                -1 : (int)effector->chain_length + 1;

        /*
         * Walk up chain starting at effector node and ending if we run out of
         * nodes, or the chain length counter reaches 0. Mark every node in
         * the chain as MARK_SECTION. If we get to the last node in the chain,
         * mark it as MARK_SPLIT only if the node is unmarked. This means that
         * nodes marked as MARK_SPLIT will be overwritten with MARK_SECTION if
         * necessary.
         */
        for (; node != NULL && chain_length_counter != 0;
             node = node->parent, chain_length_counter--)
        {
            /* Is this the last node in the chain? If so, select MARK_SPLIT */
            enum mark_e* current_mark;
            enum mark_e new_mark =
                    (chain_length_counter == 1 || node->parent == NULL)
                    ? MARK_SPLIT : MARK_SECTION;

            switch (btree_insert_or_get(marked, IK_NODE_USER_DATA(node), &new_mark, (void**)&current_mark))
            {
                case BTREE_EXISTS: {
                    /* overwrite existing mark with MARK_SECTION */
                    if (new_mark == MARK_SECTION)
                        *current_mark = MARK_SECTION;
                } break;

                case BTREE_NOT_FOUND: {
                    /* mark was inserted */
                } break;

                default: {
                    ik_log_fatal("Ran out of memory while marking involved nodes");
                    return IK_ERR_OUT_OF_MEMORY;
                }
            }
        }
    VECTOR_END_EACH

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static ikret_t
split_into_subtrees(struct vector_t* ndav_list,
                    struct ik_node_data_t* node_data,
                    const struct btree_t* marked_nodes)
{
    ikret_t status;
    int idx;
    int view_start_idx = 0;

    for (idx = 0; idx != (int)node_data->node_count; ++idx)
    {
        /*
         * A subtree is isolated from the rest of the tree if:
         * 1) Any node is marked MARK_SPLIT. These are nodes that start a chain
         *    and ended up not connecting with any other chains.
         * 2) Any node in the middle of the tree that has an effector attached.
         *    (Leaf node effectors do not split the tree)
         */
        btree_key_t uid = (uintptr_t)node_data->user_data[idx];
        enum mark_e* marking = btree_find(marked_nodes, uid);

        int is_split = (*marking == MARK_SPLIT);
        int is_embedded_effector = (node_data->effector[idx] != NULL) && (node_data->pre_order.child_count[idx] > 0);

        assert(marking);
        if ((is_split || is_embedded_effector) && idx != view_start_idx)
        {
            struct ik_node_data_view_t* ndav;
            int view_end_idx = is_split ? idx : idx + 1;

            /* Push back and init node data view */
            if ((ndav = vector_emplace(ndav_list)) == NULL)
                IK_FAIL(IK_ERR_OUT_OF_MEMORY, failed);
            if ((status = ik_node_data_view_init(ndav, node_data, view_start_idx, view_end_idx)))
            {
                vector_pop(ndav_list);
                IK_FAIL(status, failed);
            }

            /* Update start index for the next node view */
            view_start_idx = idx;
            if (is_embedded_effector && idx+1 != (int)node_data->node_count)
                if ((marking = btree_find(marked_nodes, node_data->user_data[idx+1])) && *marking == MARK_SPLIT)
                    view_start_idx++;
        }
    }

    /* Push back a node data view for the remaining nodes too (if remaining
     * nodes exist) */
    if (view_start_idx < idx - 1)
    {
        struct ik_node_data_view_t* ndav;
        if ((ndav = vector_emplace(ndav_list)) == NULL)
            IK_FAIL(IK_ERR_OUT_OF_MEMORY, failed);
        if ((status = ik_node_data_view_init(ndav, node_data, view_start_idx, idx)))
        {
            vector_pop(ndav_list);
            IK_FAIL(status, failed);
        }
    }

    return IK_OK;

    failed : VECTOR_FOR_EACH(ndav_list, struct ik_node_data_view_t, ndav)
        ik_node_data_view_deinit(ndav);
    VECTOR_END_EACH
    vector_clear_compact(ndav_list);

    return status;
}

/* ------------------------------------------------------------------------- */
static void
copy_marked_nodes_into_nda_recursive(struct ik_node_data_t* nda,
                                     struct ik_node_t* node,
                                     uint32_t base_node_idx,
                                     uint32_t* flat_idx,
                                     const enum mark_e* this_marking,
                                     const struct btree_t* marked_nodes)

{
    if (this_marking != NULL)
    {
        /*
         * Each ik_node_t object points to a ik_node_data_t object, which is
         * refcounted. After copying over the node's data, we have to point the
         * ik_node_t to the copied node data and decrement the refcount of the
         * original node data.
         *
         * Node data also points to refcounted attachments such as effectors or
         * constraints. We do not deepcopy these objects and therefore have to
         * add a reference to each of them because they're being referenced by the
         * new node data.
         */
#define X(upper, lower, type) nda->lower[*flat_idx] = node->d->lower[node->data_index];
        IK_ATTACHMENT_LIST
        IK_NODE_DATA_PROPERTIES_LIST
#undef X
#define X(upper, lower, type) IK_XINCREF(node->d->lower[node->data_index]);
        IK_ATTACHMENT_LIST                    /* addref old attachment objects */
#undef X
        IK_DECREF(node->d);                   /* (potentially) free old node data. Old attachments are decref'd but not destroyed because of line above */
        node->d = nda;                        /* point node to new node data */
        IK_INCREF(node->d);                   /* ref new node data */
        node->data_index = *flat_idx;         /* update node's index into the node data */

        /* Update index data */
        nda->pre_order.child_count[*flat_idx] = count_children_marked(node, marked_nodes);
        nda->pre_order.base_index[*flat_idx]  = base_node_idx;

        /* If this node has two or more children, it becomes the new base node for
        * the deeper nodes */
        if (nda->pre_order.child_count[*flat_idx] >= 2)
            base_node_idx = *flat_idx;

        (*flat_idx)++;
    }

    /*
     * Descend down into child nodes by preferring MARK_SECTION over MARK_SPLIT
     * and preferring MARK_SPLIT over no marking at all. This makes sure chains
     * that pass through a node with multiple children have their node data
     * placed next to each other in memory.
     */
    NODE_FOR_EACH(node, uid, child)
        if ((this_marking = btree_find(marked_nodes, IK_NODE_USER_DATA(child))) != NULL && *this_marking == MARK_SECTION)
            copy_marked_nodes_into_nda_recursive(nda, child, base_node_idx, flat_idx, this_marking, marked_nodes);
    NODE_END_EACH
    NODE_FOR_EACH(node, uid, child)
        if ((this_marking = btree_find(marked_nodes, IK_NODE_USER_DATA(child))) != NULL && *this_marking == MARK_SPLIT)
            copy_marked_nodes_into_nda_recursive(nda, child, base_node_idx, flat_idx, this_marking, marked_nodes);
    NODE_END_EACH
    NODE_FOR_EACH(node, uid, child)
        if (btree_find(marked_nodes, IK_NODE_USER_DATA(child)) == NULL)
            copy_marked_nodes_into_nda_recursive(nda, child, base_node_idx, flat_idx, NULL, marked_nodes);
    NODE_END_EACH
}
static void
copy_marked_nodes_into_nda(struct ik_node_data_t* nda,
                           struct ik_node_t* subtree_root,
                           const struct btree_t* marked_nodes)
{
    uint32_t flat_index = 0;
    enum mark_e* marking;
    marking = btree_find(marked_nodes, IK_NODE_USER_DATA(subtree_root));
    copy_marked_nodes_into_nda_recursive(nda, subtree_root, 0, &flat_index, marking, marked_nodes);

    /* Can do a buffer overrun check */
    assert(flat_index == nda->node_count);
}

/* ------------------------------------------------------------------------- */
static ikret_t
flattened_tree_create(struct ik_node_data_t** nda,
                      struct ik_node_t* root,
                      const struct btree_t* marked_nodes)
{
    ikret_t status;
    uint32_t node_count;

    /* Create new node data array into which the tree will be flattened */
    node_count = btree_count(marked_nodes);
    if ((status = ik_node_data_array_create(nda, node_count)) != IK_OK)
        return status;

    /* Tree flattening happens here */
    copy_marked_nodes_into_nda(*nda, root, marked_nodes);

    return IK_OK;
}

static void
flattened_tree_free(struct ik_node_data_t* nda)
{
    ik_node_data_free(nda);
}

/* ------------------------------------------------------------------------- */
static uint32_t
find_highest_child_count(const struct vector_t* nda_list)
{
    uint32_t max_children = 0;
    VECTOR_FOR_EACH(nda_list, struct ik_node_data_t*, pnda)
        uint32_t result = ik_node_data_find_highest_child_count(*pnda);
        max_children = result > max_children ? result : max_children;
    VECTOR_END_EACH

    return max_children;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_joblist_create(struct ik_joblist_t** joblist)
{
    *joblist = MALLOC(sizeof **joblist);
    if (*joblist == NULL)
        return IK_ERR_OUT_OF_MEMORY;

    return ik_joblist_init(*joblist);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_joblist_init(struct ik_joblist_t* joblist)
{
    if (vector_init(&joblist->ndv_list, sizeof(struct ik_node_data_view_t)) != VECTOR_OK)
    {
        ik_log_fatal("vector_init() failed on nda_list.");
        return IK_ERR_GENERIC;
    }

    joblist->highest_child_count = 0;
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void ik_joblist_deinit(struct ik_joblist_t* joblist)
{
    VECTOR_FOR_EACH(&joblist->ndv_list, struct ik_node_data_view_t, ndav)
        ik_node_data_view_deinit(ndav);
    VECTOR_END_EACH
    vector_deinit(&joblist->ndv_list);
}

/* ------------------------------------------------------------------------- */
void
ik_joblist_free(struct ik_joblist_t* joblist)
{
    ik_joblist_deinit(joblist);
    FREE(joblist);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_joblist_update(struct ik_joblist_t* joblist, struct ik_node_t* root)
{
    /*
     *
     */

    ikret_t status;
    struct vector_t effector_nodes;
    struct btree_t marked_nodes;
    struct ik_node_data_t* flattened_node_data;

    /* Create list of all nodes that have effectors attached */
    if (vector_init(&effector_nodes, sizeof(struct ik_node_t*)) != VECTOR_OK)
    {
        ik_log_fatal("vector_init() failed : Ran out of memory");
        IK_FAIL(IK_ERR_GENERIC, init_effector_nodes_failed);
    }
    if ((status = find_all_effector_nodes(&effector_nodes, root)) != IK_OK)
        IK_FAIL(status, find_effectors_failed);

    /* May not have to do anything */
    if (vector_count(&effector_nodes) == 0)
    {
        ik_log_warning("No effectors were found in the tree. Not building flattened tree structure.");
        IK_FAIL(IK_ERR_NO_EFFECTORS_FOUND, find_effectors_failed);
    }

    /* Mark all nodes that the algorithms can reach */
    if (btree_init(&marked_nodes, sizeof(enum mark_e)) != BTREE_OK)
    {
        ik_log_fatal("btree_init() failed : Ran out of memory");
        IK_FAIL(IK_ERR_GENERIC, init_marked_nodes_failed);
    }
    if ((status = mark_nodes(&marked_nodes, &effector_nodes)) != IK_OK)
        IK_FAIL(status, mark_nodes_failed);

    /* Flatten all node data */
    if ((status = flattened_tree_create(&flattened_node_data, root, &marked_nodes)) != IK_OK)
        IK_FAIL(status, flatten_tree_failed);

    /* clear old node data array views */
    VECTOR_FOR_EACH(&joblist->ndv_list, struct ik_node_data_view_t, ndav)
        ik_node_data_view_deinit(ndav);
    VECTOR_END_EACH
    vector_clear_compact(&joblist->ndv_list);

    /*
     * It's possible that chain length limits end up isolating parts of the
     * tree, splitting it into a list of "sub-trees" which must be solved
     * in-order.
     */
    if ((status = split_into_subtrees(&joblist->ndv_list, flattened_node_data, &marked_nodes)) != IK_OK)
        IK_FAIL(status, split_into_subtrees_failed);

    /* update highest child count */
    joblist->highest_child_count = find_highest_child_count(&joblist->ndv_list);

    flattened_tree_free(flattened_node_data);
    btree_deinit(&marked_nodes);
    vector_deinit(&effector_nodes);

    return IK_OK;

    split_into_subtrees_failed     : flattened_tree_free(flattened_node_data);
    flatten_tree_failed            :
    mark_nodes_failed              : btree_deinit(&marked_nodes);
    init_marked_nodes_failed       :
    find_effectors_failed          : vector_deinit(&effector_nodes);
    init_effector_nodes_failed     : return status;
}
