#include "ik/joblist.h"

#include "cstructures/memory.h"
#include "cstructures/vector.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/solver.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct marking_t
{
    enum
    {
        MARK_SECTION,
        MARK_SPLIT
    } type;

    struct ik_algorithm_t* segment_algorithm;
};

/* ------------------------------------------------------------------------- */
static uint32_t
count_children_marked(const struct ik_node_t* node,
                      const struct btree_t* marked_nodes)
{
    uint32_t count = 0;
    NODE_FOR_EACH(node, uid, child)
        if (btree_find(marked_nodes, (uintptr_t)IK_NODE_USER_DATA(node)))
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
walk_chain_and_mark(struct btree_t* marked,
                    struct ik_node_t* node,
                    struct ik_algorithm_t** chain_algorithm,
                    int chain_length_counter)
{
    /*
     * Walk up chain starting at effector node and ending if we run out of
     * nodes, or the chain length counter reaches 0. Mark every node in
     * the chain as MARK_SECTION. If we get to the last node in the chain,
     * mark it as MARK_SPLIT only if the node is unmarked. This means that
     * nodes marked as MARK_SPLIT will be overwritten with MARK_SECTION if
     * necessary.
     */
    {
        struct marking_t* current_mark;
        struct marking_t new_mark;
        /* Is this the last node in the chain? If so, select MARK_SPLIT */
        new_mark.type = (chain_length_counter == 0 || node->parent == NULL)
                        ? MARK_SPLIT : MARK_SECTION;
        new_mark.segment_algorithm = NULL;  /* Don't know algorithm yet */

        switch (btree_insert_or_get(marked, (uintptr_t)IK_NODE_USER_DATA(node), &new_mark, (void**)&current_mark))
        {
            case BTREE_EXISTS: {
                /* overwrite existing mark with MARK_SECTION */
                if (new_mark.type == MARK_SECTION)
                    current_mark->type = MARK_SECTION;
            } break;

            case BTREE_NOT_FOUND: {
                /* mark was inserted */
            } break;

            default: {
                ik_log_out_of_memory("btree_insert_or_get()");
                return IK_ERR_OUT_OF_MEMORY;
            }
        }
    }

    /* Advance to parent node recursively so the trace of nodes we processed is
     * on the stack. This lets us iterate over all of the nodes we visited in
     * reverse -- required when assigning an algorithm to each segment */
    if (chain_length_counter != 0 && node->parent != NULL)
    {
        ikret_t status;
        if ((status = walk_chain_and_mark(marked, node->parent, chain_algorithm, chain_length_counter - 1)) != IK_OK)
            return status;
    }
    /* Reached end of chain. Need to search for the next algorithm */
    else
    {
        struct ik_node_t* next;
        /* Walk further up the chain in search of the next algorithm */
        for (next = node; next != NULL; next = next->parent)
            if (IK_NODE_ALGORITHM(next) != NULL)
            {
                *chain_algorithm = IK_NODE_ALGORITHM(next);
                break;
            }
    }

    /* If the algorithm is not found by the leaf node, it means we have to search
     * back down the tree for it and unmark all nodes on the way. We walk back
     * down by walking up the callstack */
    if (*chain_algorithm == NULL)
    {
        if (IK_NODE_ALGORITHM(node) != NULL)
            *chain_algorithm = IK_NODE_ALGORITHM(node);
        else
            btree_erase(marked, (uintptr_t)IK_NODE_USER_DATA(node));
        return IK_OK;  /* don't update this node's marking with the algorithm,
                        * even if it was found. Algorithms are assigned to
                        * segments rather than individual nodes, which means
                        * we only update all parent node markings */
    }

    /* Update each segment's marking with the found algorithm */
    {
        struct marking_t* marking = btree_find(marked, (uintptr_t)IK_NODE_USER_DATA(node));
        assert(marking);
        marking->segment_algorithm = *chain_algorithm;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static ikret_t
mark_nodes(struct btree_t* marked, const struct vector_t* effector_nodes)
{
    /*
     * Iterate the chain of nodes starting at each effector node and ending
     * at the specified chain length of the effector, mark every node on the
     * way.
     */
    VECTOR_FOR_EACH(effector_nodes, struct ik_node_t*, p_effector_node)
        ikret_t status;
        struct ik_node_t* node                 = *p_effector_node;
        const struct ik_effector_t* effector   = IK_NODE_EFFECTOR(node);
        struct ik_algorithm_t* chain_algorithm = NULL;
        int chain_length_counter               = (int)effector->chain_length != 0 ?
                                                 (int)effector->chain_length : -1;

        if ((status = walk_chain_and_mark(marked, node, &chain_algorithm, chain_length_counter)) != IK_OK)
            return status;
        if (chain_algorithm == NULL)
            return IK_ERR_NO_ALGORITHMS_FOUND;
    VECTOR_END_EACH

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static void
copy_marked_nodes_into_nda_recursive(struct ik_node_data_t* nda,
                                     struct ik_node_t* node,
                                     uint32_t base_node_idx,
                                     uint32_t parent_node_idx,
                                     uint32_t chain_depth,
                                     uint32_t* flat_idx,
                                     const struct marking_t* this_marking,
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
        nda->base_idx[*flat_idx]    = base_node_idx;
        nda->parent_idx[*flat_idx]  = parent_node_idx;
        nda->child_count[*flat_idx] = count_children_marked(node, marked_nodes);
        nda->chain_depth[*flat_idx] = chain_depth;

        /* If this node has two or more children, it becomes the new base node for
        * the deeper nodes */
        if (nda->child_count[*flat_idx] >= 2)
        {
            base_node_idx = *flat_idx;
            chain_depth++;
        }

        parent_node_idx = *flat_idx;
        (*flat_idx)++;
    }

    /*
     * Descend down into child nodes by preferring MARK_SECTION over MARK_SPLIT
     * and preferring MARK_SPLIT over no marking at all. This makes sure chains
     * that pass through a node with multiple children have their node data
     * placed next to each other in memory.
     */
    NODE_FOR_EACH(node, uid, child)
        struct marking_t* child_marking = btree_find(marked_nodes, (uintptr_t)IK_NODE_USER_DATA(child));
        if (child_marking && this_marking && child_marking->segment_algorithm == this_marking->segment_algorithm)
            copy_marked_nodes_into_nda_recursive(nda, child, base_node_idx, parent_node_idx, chain_depth, flat_idx, child_marking, marked_nodes);
    NODE_END_EACH
    NODE_FOR_EACH(node, uid, child)
        struct marking_t* child_marking = btree_find(marked_nodes, (uintptr_t)IK_NODE_USER_DATA(child));
        if (child_marking == NULL || this_marking == NULL || child_marking->segment_algorithm != this_marking->segment_algorithm)
            copy_marked_nodes_into_nda_recursive(nda, child, base_node_idx, parent_node_idx, chain_depth, flat_idx, child_marking, marked_nodes);
    NODE_END_EACH
}
static void
copy_marked_nodes_into_nda(struct ik_node_data_t* nda,
                           struct ik_node_t* subtree_root,
                           const struct btree_t* marked_nodes)
{
    uint32_t flat_index = 0;
    struct marking_t* marking;
    marking = btree_find(marked_nodes, (uintptr_t)IK_NODE_USER_DATA(subtree_root));
    /* marking may be NULL, doesn't matter */
    copy_marked_nodes_into_nda_recursive(nda,
                                         subtree_root,
                                         0,  /* base node index */
                                         -1, /* parent node index */
                                         0,  /* chain depth */
                                         &flat_index,
                                         marking,
                                         marked_nodes);

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

/* ------------------------------------------------------------------------- */
static void
flattened_tree_free(struct ik_node_data_t* nda)
{
    ik_node_data_free(nda);
}

/* ------------------------------------------------------------------------- */
static ikret_t
allocate_solvers_to_subtrees(struct ik_joblist_t* joblist,
                             struct ik_node_data_t* node_data,
                             const struct btree_t* marked_nodes)
{
    ikret_t status;
    int idx;
    int subbase_idx = 0;
    int chain_start_idx = 1;
    struct ik_algorithm_t* last_algorithm = NULL;

    assert(node_data->node_count > 1);

    for (idx = 1; idx != (int)node_data->node_count; ++idx)
    {
        /*
         * A subtree is isolated from the rest of the tree if:
         * 1) Any node is marked MARK_SPLIT. These are nodes that start a chain
         *    and ended up not connecting with any other chains.
         * 2) Any node in the middle of the tree that has an effector attached.
         *    (Leaf node effectors do not split the tree)
         */
        btree_key_t uid = (uintptr_t)node_data->user_data[idx];
        struct marking_t* marking = btree_find(marked_nodes, uid);

        int is_split = (marking->type == MARK_SPLIT || last_algorithm != marking->segment_algorithm);
        int is_embedded_effector = (node_data->effector[idx] != NULL) && (node_data->child_count[idx] > 0);

        if (last_algorithm != marking->segment_algorithm)
        {
            subbase_idx = idx - 1;
            last_algorithm = marking->segment_algorithm;
        }

        if ((is_split || is_embedded_effector) && idx != chain_start_idx)
        {
            struct ik_solver_t* solver;
            int view_end_idx = is_split ? idx : idx + 1;

            /* Push back and init node data view */
            if ((status = ik_solver_create(&solver, marking->segment_algorithm, node_data, subbase_idx, chain_start_idx, view_end_idx)) != IK_OK)
                IK_FAIL(status, failed);
            if (vector_push(&joblist->solver_list, &solver) != VECTOR_OK)
            {
                ik_solver_free(solver);
                IK_FAIL(IK_ERR_OUT_OF_MEMORY, failed);
            }

            /* Update start index for the next node view */
            chain_start_idx = idx + 1;
            if (is_embedded_effector && idx+1 != (int)node_data->node_count)
                if ((marking = btree_find(marked_nodes, (uintptr_t)node_data->user_data[idx+1])) && marking->type == MARK_SPLIT)
                    chain_start_idx++;
        }
    }

    /* Add a solver for the remaining nodes too (if remaining nodes exist) */
    if (chain_start_idx < idx - 1)
    {
        struct ik_solver_t* solver;
        struct marking_t* marking = btree_find(marked_nodes, (uintptr_t)node_data->user_data[chain_start_idx]);
        if ((status = ik_solver_create(&solver, marking->segment_algorithm, node_data, subbase_idx, chain_start_idx, idx)) != IK_OK)
            IK_FAIL(status, failed);
        if (vector_push(&joblist->solver_list, &solver) != VECTOR_OK)
        {
            ik_solver_free(solver);
            IK_FAIL(IK_ERR_OUT_OF_MEMORY, failed);
        }
    }

    return IK_OK;

    failed : VECTOR_FOR_EACH(&joblist->solver_list, struct ik_solver_t*, solver)
        ik_solver_free(*solver);
    VECTOR_END_EACH
    vector_clear_compact(&joblist->solver_list);

    return status;
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
    if (vector_init(&joblist->solver_list, sizeof(struct ik_solver_t*)) != VECTOR_OK)
    {
        ik_log_fatal("vector_init() failed on solver_list.");
        return IK_ERR_GENERIC;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void ik_joblist_deinit(struct ik_joblist_t* joblist)
{
    VECTOR_FOR_EACH(&joblist->solver_list, struct ik_solver_t*, solver)
        ik_solver_free(*solver);
    VECTOR_END_EACH
    vector_deinit(&joblist->solver_list);
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
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, init_effector_nodes_failed);
    }
    if ((status = find_all_effector_nodes(&effector_nodes, root)) != IK_OK)
        IK_FAIL(status, find_effectors_failed);

    /* May not have to do anything */
    if (vector_count(&effector_nodes) == 0)
    {
        ik_log_warning("No effectors were found in the tree. Not building flattened tree structure.");
        IK_FAIL(IK_ERR_NO_EFFECTORS_FOUND, find_effectors_failed);
    }

    /* Mark all nodes that the effectors can reach */
    if (btree_init(&marked_nodes, sizeof(struct marking_t)) != BTREE_OK)
    {
        ik_log_fatal("btree_init() failed : Ran out of memory");
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, init_marked_nodes_failed);
    }
    if ((status = mark_nodes(&marked_nodes, &effector_nodes)) != IK_OK)
        IK_FAIL(status, mark_nodes_failed);

    /* Flatten all node data */
    if ((status = flattened_tree_create(&flattened_node_data, root, &marked_nodes)) != IK_OK)
        IK_FAIL(status, flatten_tree_failed);

    /* clear old solvers */
    VECTOR_FOR_EACH(&joblist->solver_list, struct ik_solver_t*, solver)
        ik_solver_free(*solver);
    VECTOR_END_EACH
    vector_clear_compact(&joblist->solver_list);

    /*
     * It's possible that chain length limits end up isolating parts of the
     * tree, splitting it into a list of "sub-trees" which must be solved
     * in-order.
     */
    if ((status = allocate_solvers_to_subtrees(joblist, flattened_node_data, &marked_nodes)) != IK_OK)
        IK_FAIL(status, split_into_subtrees_failed);

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
