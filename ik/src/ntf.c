#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/ntf.h"
#include "ik/node.h"
#include "ik/node_data.h"
#include "ik/vector.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

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
        if ((status = find_all_effector_nodes(result, child)) != IK_OK)
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
        for (; node != NULL && chain_length_counter != 0;
             node = node->parent, chain_length_counter--)
        {
            /* Is this the last node in the chain? If so, select MARK_BASE */
            enum mark_e new_mark =
                    (chain_length_counter == 0 || node->parent == NULL)
                    ? MARK_BASE : MARK_SECTION;

            enum mark_e* current_marking = (enum mark_e*)btree_find_ptr(marked, ik_node_get_uid(node));
            if (current_marking != NULL)  /* node already marked? */
            {
                if (new_mark == MARK_SECTION)
                    *current_marking = MARK_SECTION;
            }
            else  /* Node isn't marked yet */
            {
                ikret_t status;
                if ((status = btree_insert(marked, ik_node_get_uid(node),
                    (void*)(intptr_t)new_mark)) != IK_OK)
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
     * Any node marked MARK_BASE is the root node of an isolated tree.
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
static void
count_marked_nodes_in_subtree(uint32_t* total_count,
                              uint32_t* max_children,
                              const struct ik_node_t* node,
                              const struct btree_t* marked_nodes)
{
    uint32_t child_count = 0;
    NODE_FOR_EACH(node, uid, child)
        enum mark_e* marking =
            (enum mark_e*)btree_find_ptr(marked_nodes, ik_node_get_uid(child));
        if (marking != NULL && *marking == MARK_SECTION)
        {
            child_count++;
            (*total_count)++;
            count_marked_nodes_in_subtree(total_count, max_children, child, marked_nodes);
        }
    NODE_END_EACH

    if (*max_children < child_count)
        *max_children = child_count;
}
#define count_marked_nodes_in_subtree(total_count, max_children, node, marked_nodes) do { \
        *total_count = 0; \
        *max_children = 0; \
        count_marked_nodes_in_subtree(total_count, max_children, node, marked_nodes); \
        (*total_count)++; /* account for root node */ \
    } while (0)

/* ------------------------------------------------------------------------- */
static void
calculate_indices_recursive(struct ik_ntf_t* ntf,
                            uint32_t* pre_counter,
                            uint32_t* post_counter,
                            const struct ik_node_t* node,
                            const struct btree_t* marked_nodes)
{
    uint32_t marked_child_node_count;

    /*
     * Count the child nodes that are marked. Have to do this before recursing
     * into the children, because doing that would update *pre_counter.
     */
    marked_child_node_count = 0;
    NODE_FOR_EACH(node, uid, child)
        enum mark_e* marking =
                (enum mark_e*)btree_find_ptr(marked_nodes, ik_node_get_uid(child));
        if (marking != NULL && *marking == MARK_SECTION)
            marked_child_node_count++;
    NODE_END_EACH

    /* Set "pre-order" indices */
    ntf->indices[*pre_counter].pre = (uint32_t)(node->node_data - ntf->node_data);
    ntf->indices[*pre_counter].pre_child_count = marked_child_node_count;
    (*pre_counter)++;

    /* Recurse into children */
    NODE_FOR_EACH(node, uid, child)
        enum mark_e* marking =
                (enum mark_e*)btree_find_ptr(marked_nodes, ik_node_get_uid(child));
        if (marking != NULL && *marking == MARK_SECTION)
            calculate_indices_recursive(ntf,
                                        pre_counter,
                                        post_counter,
                                        child,
                                        marked_nodes);
    NODE_END_EACH

    /* Set "post-order" indices */
    ntf->indices[*post_counter].post = (uint32_t)(node->node_data - ntf->node_data);
    ntf->indices[*post_counter].post_child_count = marked_child_node_count;
    (*post_counter)++;
}
static void
calculate_indices(struct ik_ntf_t* ntf,
                  const struct ik_node_t* subtree_root,
                  const struct btree_t* marked_nodes)
{
    uint32_t pre_counter = 0;
    uint32_t post_counter = 0;
    calculate_indices_recursive(ntf,
                                &pre_counter,
                                &post_counter,
                                subtree_root,
                                marked_nodes);
}

/* ------------------------------------------------------------------------- */
static void
copy_marked_nodes_into_ntf_recursive(struct ik_node_data_t** buffer_dest,
                                     struct ik_node_t* node,
                                     struct ik_refcount_t* shared_array_refcount,
                                     const struct btree_t* marked_nodes)
{
    /* Only copy nodes that are marked as MARK_SECTION */
    enum mark_e* marking =
            (enum mark_e*)btree_find_ptr(marked_nodes, ik_node_get_uid(node));
    if (marking == NULL || *marking != MARK_SECTION)
        return;

    /*
     * Each ik_node_t points to a ik_node_data_t structure, which is refcounted.
     * After memcopying the node data, we have to point the ik_node_t to the
     * copied node data and decrement the refcount of the original node data.
     * That way, the user of the library can edit node properties even after
     * the node tree was optimized.
     */
    memcpy(*buffer_dest, node->node_data, sizeof(struct ik_node_data_t));
    ik_node_data_ref_members(node->node_data);
    IK_DECREF(node->node_data);
    node->node_data = (*buffer_dest)++;

    /*
     * The refcount of the copied data is invalidated. We must point it to the
     * buffer's refcount and incref it appropriately.
     */
    node->node_data->refcount = shared_array_refcount;
    IK_INCREF(node->node_data);

    NODE_FOR_EACH(node, uid, child)
        copy_marked_nodes_into_ntf_recursive(buffer_dest, child, shared_array_refcount, marked_nodes);
    NODE_END_EACH
}
static ikret_t
copy_marked_nodes_into_ntf(struct ik_ntf_t* ntf,
                           struct ik_node_t* subtree_root,
                           const struct btree_t* marked_nodes)
{
    ikret_t status;
    struct ik_node_data_t* buffer_dest;
    struct ik_refcount_t* buffer_refcount;

    /*
     * Each ik_node_t references a ik_node_data_t instance (which is refcounted).
     * After memcpy'ing this node_data instance, we have to give the copied data
     * its own refcount (otherwise it will be using the refcount of the original data).
     *
     * We also have to point the ik_node_t to the copied node data and
     * decrement the refcount of the original node data appropriately. This way,
     * the user of the library can continue to edit node properties without
     * knowing the data was moved to a new memory location.
     */
    memcpy(ntf->node_data, subtree_root->node_data, sizeof(struct ik_node_data_t));
    if ((status = ik_refcount_create(
            &ntf->node_data->refcount,
            subtree_root->node_data->refcount->destruct,
            ntf->node_count)) != IK_OK)
        return status;

    /* deref old node data, update pointer to new node data for root node */
    ik_node_data_ref_members(subtree_root->node_data);
    IK_DECREF(subtree_root->node_data);
    subtree_root->node_data = ntf->node_data;

    /*
     * With the root node copied and its refcount initialized, we can recursively
     * copy the rest of the nodes into the buffer.
     */
    buffer_dest = ntf->node_data + 1;
    buffer_refcount = subtree_root->node_data->refcount;
    NODE_FOR_EACH(subtree_root, uid, child)
        copy_marked_nodes_into_ntf_recursive(&buffer_dest, child, buffer_refcount, marked_nodes);
    NODE_END_EACH

    /* Can do a buffer overrun check */
    assert(buffer_dest - ntf->node_data == ntf->node_count);

    calculate_indices(ntf, subtree_root, marked_nodes);

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static ikret_t
process_subtree(struct vector_t* ntf_list,
                struct ik_node_t* subtree_root,
                const struct btree_t* marked_nodes)
{
    ikret_t status;

    struct ik_ntf_t* ntf = vector_emplace(ntf_list);
    if (ntf == NULL)
    {
        ik_log_fatal("Failed to emplace new flattened tree structure: Ran out of memory");
        FAIL(IK_ERR_OUT_OF_MEMORY, emplace_failed);
    }

    if ((status = ik_ntf_construct(ntf, subtree_root, marked_nodes)) != IK_OK)
        FAIL(status, ntf_construct_failed);

    return IK_OK;

    ntf_construct_failed     : vector_pop(ntf_list);
    emplace_failed           : return status;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_ntf_create(struct ik_ntf_t** ntf,
              struct ik_node_t* subtree_root,
              const struct btree_t* marked_nodes)
{
    ikret_t status;
    assert(ntf);
    assert(subtree_root);
    assert(marked_nodes);

    *ntf = MALLOC(sizeof **ntf);
    if (*ntf == NULL)
    {
        ik_log_fatal("Failed to allocate NTF: Ran out of memory");
        FAIL(IK_ERR_OUT_OF_MEMORY, alloc_ntf_failed);
    }

    if ((status = ik_ntf_construct(*ntf, subtree_root, marked_nodes)) != IK_OK)
        FAIL(status, construct_ntf_failed);

    return IK_OK;

    construct_ntf_failed : FREE(*ntf);
    alloc_ntf_failed     : return status;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_ntf_construct(struct ik_ntf_t* ntf,
                 struct ik_node_t* subtree_root,
                 const struct btree_t* marked_nodes)
{
    ikret_t status;
    uint32_t max_children;
    assert(ntf);
    assert(subtree_root);
    assert(marked_nodes);

    memset(ntf, 0, sizeof *ntf);

    count_marked_nodes_in_subtree(&ntf->node_count, &max_children, subtree_root, marked_nodes);

    /* Contiguous array for holding all ik_node_data_t instances of all nodes */
    ntf->node_data = MALLOC(sizeof(struct ik_node_data_t) * ntf->node_count);
    if (ntf->node_data == NULL)
    {
        ik_log_fatal("Failed to allocate NTF: Ran out of memory");
        FAIL(IK_ERR_OUT_OF_MEMORY, alloc_nodes_buffer_failed);
    }

    /*
     * Index data to allow iterating the tree in pre- and post-order after it
     * was flattened.
     */
    ntf->indices = MALLOC(sizeof(struct ik_ntf_index_data_t) * ntf->node_count);
    if (ntf->indices == NULL)
    {
        ik_log_fatal("Failed to allocate NTF index data: Ran out of memory");
        FAIL(IK_ERR_OUT_OF_MEMORY, alloc_index_buffer_failed);
    }

    /*
     * The solver needs a small stack to push/pop transformations as it
     * iterates the tree.
     * TODO: Add support for alloca(). If the stack is small enough and the
     * platform supports alloca(), leave this as NULL.
     */
    if ((ntf->scratch_buffer = MALLOC(sizeof(union ik_transform_t) * max_children)) == NULL)
    {
        ik_log_fatal("Failed to allocate scratch buffer: Ran out of memory");
        FAIL(IK_ERR_OUT_OF_MEMORY, alloc_scratch_buffer_failed);
    }

    /* Actual flattening of the tree happens here */
    if ((status = copy_marked_nodes_into_ntf(ntf, subtree_root, marked_nodes)) != IK_OK)
        FAIL(status, copy_nodes_failed);

    /*
     * The NTF structure needs to hold a reference to the node data in case all
     * ik_node_t instances are destroyed.
     */
    IK_INCREF(ntf->node_data);

    return IK_OK;

    copy_nodes_failed           : FREE(ntf->scratch_buffer);
    alloc_scratch_buffer_failed : FREE(ntf->indices);
    alloc_index_buffer_failed   : FREE(ntf->node_data);
    alloc_nodes_buffer_failed   : return status;
}

/* ------------------------------------------------------------------------- */
void
ik_ntf_destruct(struct ik_ntf_t* ntf)
{
    if (ntf->scratch_buffer)
        FREE(ntf->scratch_buffer);
    FREE(ntf->indices);
    IK_DECREF(ntf->node_data);
}

/* ------------------------------------------------------------------------- */
void
ik_ntf_destroy(struct ik_ntf_t* ntf)
{
    ik_ntf_destruct(ntf);
    FREE(ntf);
}


/* ------------------------------------------------------------------------- */
ikret_t
ik_ntf_list_from_nodes(struct vector_t** ntf_list, struct ik_node_t* root)
{
    ikret_t status;
    struct vector_t effector_nodes;
    struct vector_t subtree_list;
    struct btree_t marked_nodes;

    /* Create list of all nodes that have effectors attached */
    vector_construct(&effector_nodes, sizeof(struct ik_node_t*));
    if ((status = find_all_effector_nodes(&effector_nodes, root)) != IK_OK)
        FAIL(status, find_effectors_failed);
    if (vector_count(&effector_nodes) == 0)
    {
        ik_log_warning("No effectors were found in the tree. Not building flattened tree structure.");
        FAIL(IK_ERR_NO_EFFECTORS_FOUND, find_effectors_failed);
    }

    /* Mark all nodes that the solver can reach */
    btree_construct(&marked_nodes);
    if ((status = mark_nodes(&marked_nodes, &effector_nodes)) != IK_OK)
        FAIL(status, mark_nodes_failed);

    /*
     * It's possible that chain length limits end up isolating parts of the
     * tree, splitting it into a list of "sub-trees" which must be solved
     * "in-order" (LNR).
     */
    vector_construct(&subtree_list, sizeof(struct ik_node_t*));
    if ((status = split_into_subtrees(&subtree_list, root, &marked_nodes)) != IK_OK)
        FAIL(status, split_into_islands_failed);

    /*
     * Go through each subtree and flatten it.
     */
    if ((status = vector_create(ntf_list, sizeof(struct ik_ntf_t))) != IK_OK)
        FAIL(status, ntf_list_create_vector_failed);
    VECTOR_FOR_EACH(&subtree_list, struct ik_node_t*, node)
        if ((status = process_subtree(*ntf_list, *node, &marked_nodes)) != IK_OK)
            FAIL(status, process_subtrees_failed);
    VECTOR_END_EACH

    vector_clear_free(&subtree_list);
    btree_clear_free(&marked_nodes);
    vector_clear_free(&effector_nodes);

    return IK_OK;

    process_subtrees_failed        : vector_destroy(*ntf_list);
    ntf_list_create_vector_failed  :
    split_into_islands_failed      : vector_clear_free(&subtree_list);
    mark_nodes_failed              : btree_clear_free(&marked_nodes);
    find_effectors_failed          : vector_clear_free(&effector_nodes);
    return status;
}

/* ------------------------------------------------------------------------- */
void
ik_ntf_list_destroy(struct vector_t* ntf_list)
{
    VECTOR_FOR_EACH(ntf_list, struct ik_ntf_t, ntf)
        ik_ntf_destruct(ntf);
    VECTOR_END_EACH
    vector_destroy(ntf_list);
}
