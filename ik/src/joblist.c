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


enum ik_marking
{
    MARK_INVALID,
    MARK_SECTION,
    MARK_BEGIN,
    MARK_END,
    MARK_BEGIN_AND_END
};

/* ------------------------------------------------------------------------- */
static uint32_t
count_children_marked(const struct ik_node* node,
                      const struct btree_t* marked_nodes)
{
    uint32_t count = 0;
    NODE_FOR_EACH(node, uid, child)
        if (btree_find(marked_nodes, node->user.guid))
            count++;
    NODE_END_EACH

    return count;
}

/* ------------------------------------------------------------------------- */
static int
find_all_effector_nodes(struct vector_t* result, const struct ik_node* node)
{
    NODE_FOR_EACH(node, user_data, child)
        if (find_all_effector_nodes(result, child) != 0)
            return -1;
    NODE_END_EACH

    if (node->effector != NULL)
        if (vector_push(result, &node) != VECTOR_OK)
            return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */
static ikret
walk_chain_and_mark(struct btree_t* marked,
                    const struct ik_node* node,
                    struct ik_algorithm** chain_algorithm,
                    int chain_length_counter)
{
    /*
     * Walk up chain starting at effector node and ending if we run out of
     * nodes, or the chain length counter reaches 0. Mark every node in
     * the chain as MARK_SECTION. If we get to the last node in the chain,
     * mark it as MARK_END only if the node is unmarked. This means that
     * nodes marked as MARK_END will be overwritten with MARK_SECTION if
     * necessary.
     */
    {
        struct marking_t* current_mark;
        struct marking_t new_mark;
        /* Is this the last node in the chain? If so, select MARK_END */
        new_mark.type = (chain_length_counter == 0 || node->parent == NULL)
                        ? MARK_END : MARK_SECTION;
        new_mark.segment_algorithm = NULL;  /* Don't know algorithm yet */

        switch (btree_insert_or_get(marked, node->user.guid, &new_mark, (void**)&current_mark))
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
     * reverse -- required when assigning a solver to each segment */
    if (chain_length_counter != 0 && node->parent != NULL)
    {
        ikret status;
        if ((status = walk_chain_and_mark(marked, node->parent, chain_algorithm, chain_length_counter - 1)) != IK_OK)
            return status;
    }
    /* Reached end of chain. Need to search for the next algorithm */
    else
    {
        const struct ik_node* next;
        /* Walk further up the chain in search of the next algorithm */
        for (next = node; next != NULL; next = next->parent)
            if (next->algorithm != NULL)
            {
                *chain_algorithm = next->algorithm;
                break;
            }
    }

    /* If the algorithm is not found by the leaf node, it means we have to search
     * back down the tree for it and unmark all nodes on the way. We walk back
     * down by walking up the callstack */
    if (*chain_algorithm == NULL)
    {
        if (node->algorithm != NULL)
            *chain_algorithm = node->algorithm;
        else
            btree_erase(marked, node->user.guid);
        return IK_OK;  /* don't update this node's marking with the algorithm,
                        * even if it was found. Algorithms are assigned to
                        * segments rather than individual nodes, which means
                        * we only update all parent node markings */
    }

    /* Update each segment's marking with the found algorithm */
    {
        struct marking_t* marking = btree_find(marked, node->user.guid);
        assert(marking);
        marking->segment_algorithm = *chain_algorithm;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static int
mark_nodes(struct btree_t* marked, const struct vector_t* effector_nodes)
{
    /*
     * Iterate the chain of nodes starting at each effector node and ending
     * at the specified chain length of the effector, mark every node on the
     * way.
     */
    VECTOR_FOR_EACH(effector_nodes, const struct ik_node*, p_effector_node)
        const struct ik_node* node = *p_effector_node;
        const struct ik_effector* effector = node->effector;
        int chain_length_counter = (int)effector->chain_length != 0 ?
                                   (int)effector->chain_length : -1;

        /*
         * Walk up chain starting at effector node and ending if we run out of
         * nodes, or the chain length counter reaches 0. Mark every node in
         * the chain as MARK_SECTION. If we get to the last node in the chain,
         * mark it as MARK_END only if the node is unmarked. This means that
         * nodes marked as MARK_END will be overwritten with MARK_SECTION if
         * necessary.
         */
        while (1)
        {
#define is_end_of_chain() \
            (chain_length_counter == 0 || node->parent == NULL)
#define has_children() \
            (ik_node_child_count(node) > 0)
#define has_algorithm() \
            (node->algorithm != NULL)
#define has_effector() \
            (node->effector != NULL)

            enum ik_marking* current_mark;
            const enum ik_marking lookup_mark[16] = {
                MARK_INVALID,
                MARK_INVALID,
                MARK_SECTION,
                MARK_BEGIN,
                MARK_END,
                MARK_END,
                MARK_BEGIN_AND_END,
                MARK_BEGIN_AND_END,
                MARK_INVALID,
                MARK_INVALID,
                MARK_SECTION,
                MARK_BEGIN,
                MARK_BEGIN,
                MARK_BEGIN_AND_END,
                MARK_BEGIN_AND_END,
                MARK_BEGIN_AND_END
            };

            const uint8_t mark_idx =
                (is_end_of_chain() << 0) |
                (has_children()    << 1) |
                (has_effector()    << 2) |
                (has_algorithm()   << 3);

            enum ik_marking new_mark = lookup_mark[mark_idx];

            switch (btree_insert_or_get(marked, node->user.guid, &new_mark, (void**)&current_mark))
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
                    ik_log_out_of_memory("btree_insert_or_get()");
                    return -1;
                }
            }

            if (chain_length_counter == 0 || node->parent == NULL)
                break;
        }
    VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
static ikret
assign_solvers_to_subtrees(struct ik_joblist* joblist,
                           const struct ik_node* node,
                           const struct btree_t* marked_nodes)
{
    ikret status;
    int idx;
    int subbase_idx = 0;
    int chain_start_idx = 1;
    struct ik_algorithm* last_algorithm = NULL;

    assert(node_data->node_count > 1);

    for (idx = 1; idx != (int)node_data->node_count; ++idx)
    {
        /*
         * A subtree is isolated from the rest of the tree if:
         * 1) any node is marked MARK_END. These are nodes that start a chain
         *    and ended up not connecting with any other chains.
         * 2) any node in the middle of the tree has an effector attached.
         *    (Leaf node effectors are obviously an exception)
         */
        btree_key_t guid = node->user.guid;
        struct marking_t* marking = btree_find(marked_nodes, guid);

        int is_split = (marking->type == MARK_END || last_algorithm != marking->segment_algorithm);
        int is_embedded_effector = (node_data->effector[idx] != NULL) && (node_data->child_count[idx] > 0);

        if (last_algorithm != marking->segment_algorithm)
        {
            subbase_idx = idx - 1;
            last_algorithm = marking->segment_algorithm;
        }

        if ((is_split || is_embedded_effector) && idx != chain_start_idx)
        {
            struct ik_solver* solver;
            int view_end_idx = is_split ? idx : idx + 1;

            /* Push back and init node data view */
            if ((status = ik_solver_create(&solver, marking->segment_algorithm, node_data, subbase_idx, chain_start_idx, view_end_idx)) != IK_OK)
                IK_FAIL(status, failed);
            if (vector_push(&joblist->solvers, &solver) != VECTOR_OK)
            {
                ik_solver_free(solver);
                IK_FAIL(IK_ERR_OUT_OF_MEMORY, failed);
            }

            /* Update start index for the next node view */
            chain_start_idx = idx + 1;
            if (is_embedded_effector && idx+1 != (int)node_data->node_count)
                if ((marking = btree_find(marked_nodes, (uintptr_t)node_data->user_data[idx+1])) && marking->type == MARK_END)
                    chain_start_idx++;
        }
    }

    /* Add a solver for the remaining nodes too (if remaining nodes exist) */
    if (chain_start_idx < idx - 1)
    {
        struct ik_solver* solver;
        struct marking_t* marking = btree_find(marked_nodes, (uintptr_t)node_data->user_data[chain_start_idx]);
        if ((status = ik_solver_create(&solver, marking->segment_algorithm, node_data, subbase_idx, chain_start_idx, idx)) != IK_OK)
            IK_FAIL(status, failed);
        if (vector_push(&joblist->solvers, &solver) != VECTOR_OK)
        {
            ik_solver_free(solver);
            IK_FAIL(IK_ERR_OUT_OF_MEMORY, failed);
        }
    }

    return IK_OK;

    failed : VECTOR_FOR_EACH(&joblist->solvers, struct ik_solver*, solver)
        ik_solver_free(*solver);
    VECTOR_END_EACH
    vector_clear_compact(&joblist->solvers);

    return status;
}

/* ------------------------------------------------------------------------- */
ikret
ik_joblist_create(struct ik_joblist** joblist)
{
    *joblist = MALLOC(sizeof **joblist);
    if (*joblist == NULL)
        return IK_ERR_OUT_OF_MEMORY;

    return ik_joblist_init(*joblist);
}

/* ------------------------------------------------------------------------- */
ikret
ik_joblist_init(struct ik_joblist* joblist)
{
    if (vector_init(&joblist->solvers, sizeof(struct ik_solver*)) != VECTOR_OK)
    {
        ik_log_fatal("vector_init() failed on solvers.");
        return IK_ERR_GENERIC;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void ik_joblist_deinit(struct ik_joblist* joblist)
{
    VECTOR_FOR_EACH(&joblist->solvers, struct ik_solver*, solver)
        ik_solver_free(*solver);
    VECTOR_END_EACH
    vector_deinit(&joblist->solvers);
}

/* ------------------------------------------------------------------------- */
ikret
ik_joblist_update(struct ik_joblist* joblist, const struct ik_node* root)
{
    /*
     *
     */

    ikret status;
    struct vector_t effector_nodes;
    struct btree_t marked_nodes;

    /* Create list of all nodes that have effectors attached */
    if (vector_init(&effector_nodes, sizeof(struct ik_node*)) != VECTOR_OK)
    {
        ik_log_out_of_memory("vector_init()");
        status = IK_ERR_OUT_OF_MEMORY;
        goto init_effector_nodes_failed;
    }
    if ((status = find_all_effector_nodes(&effector_nodes, root)) != IK_OK)
        goto find_effectors_failed;

    /* May not have to do anything */
    if (vector_count(&effector_nodes) == 0)
    {
        ik_log_printf(IK_WARN, "No effectors were found in the tree. Joblist is empty.");
        status = IK_ERR_NO_EFFECTORS_FOUND;
        goto find_effectors_failed;
    }

    /* Mark all nodes that the effectors can reach */
    if (btree_init(&marked_nodes, sizeof(struct marking_t)) != BTREE_OK)
    {
        ik_log_out_of_memory("btree_init()");
        status = IK_ERR_OUT_OF_MEMORY;
        goto init_marked_nodes_failed;
    }
    if ((status = mark_nodes(&marked_nodes, &effector_nodes)) != IK_OK)
        goto mark_nodes_failed;

    /* clear old solvers */
    VECTOR_FOR_EACH(&joblist->solvers, struct ik_solver*, solver)
        ik_solver_free(*solver);
    VECTOR_END_EACH
    vector_clear_compact(&joblist->solvers);

    /*
     * It's possible that chain length limits end up isolating parts of the
     * tree, splitting it into a list of "sub-trees" which must be solved
     * in-order.
     */
    if ((status = assign_solvers_to_subtrees(joblist, root, &marked_nodes)) != IK_OK)
        goto split_into_subtrees_failed;

    btree_deinit(&marked_nodes);
    vector_deinit(&effector_nodes);

    return IK_OK;

    split_into_subtrees_failed     :
    mark_nodes_failed              : btree_deinit(&marked_nodes);
    init_marked_nodes_failed       :
    find_effectors_failed          : vector_deinit(&effector_nodes);
    init_effector_nodes_failed     : return status;
}
