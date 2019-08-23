#include "cstructures/memory.h"
#include "ik/log.h"
#include "ik/node_data.h"
#include "ik/effector.h"
#include "ik/constraint.h"
#include "ik/pole.h"
#include <assert.h>
#include <string.h>
#include <stddef.h>

/* ------------------------------------------------------------------------- */
static void
deinit_node_data(struct ik_node_data_t* node_data)
{
    size_t i = node_data->node_count;
    while (i--)
    {
#define X(upper, lower, type) IK_XDECREF(node_data->lower[i]);
        IK_ATTACHMENT_LIST
#undef X
    }
}

/* ------------------------------------------------------------------------- */
static uintptr_t
align_to_cpu_word_size(uintptr_t offset)
{
    if ((offset & 0x7) == 0)
        return offset;
    return (offset & ~0x7) + 8;
}

/* ------------------------------------------------------------------------- */
static struct ik_node_data_t*
determine_zla_size_and_malloc(uintptr_t node_count)
{
    ikret_t status;
    struct ik_node_data_t* node_data;
    struct ik_node_data_t offsets;
    uintptr_t zla_size = 0;
    uintptr_t buffer_alignment;

#define ADD_TO_SIZE(name) \
    zla_size = align_to_cpu_word_size(zla_size); \
    offsets.name = (void*)zla_size; \
    zla_size += sizeof(*node_data->name) * node_count;

    /* Used to align the buffer that is added to the end of the structure */
    buffer_alignment = sizeof(struct ik_node_data_t);
    buffer_alignment = align_to_cpu_word_size(buffer_alignment) - buffer_alignment;

    /*
     * Calculates the required buffer size for all of the members (including
     * attachments) and store the aligned offsets to the beginning of each
     * member in "offsets"
     */
#define X(upper, lower, type) ADD_TO_SIZE(lower)
    IK_ATTACHMENT_LIST
    IK_NODE_DATA_PROPERTIES_LIST
#undef X

    /* Space for index data */
    ADD_TO_SIZE(base_idx)
    ADD_TO_SIZE(parent_idx)
    ADD_TO_SIZE(child_count)
    ADD_TO_SIZE(chain_depth)

    /* Allocate and zero init structure */
    node_data = MALLOC(sizeof(*node_data) + zla_size + buffer_alignment);
    if (node_data == NULL)
    {
        ik_log_fatal("Failed to allocate node_data: Ran out of memory");
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, alloc_node_data_failed);
    }
    memset(node_data, 0, sizeof(*node_data) + zla_size + buffer_alignment);

#define ADD_OFFSET(name) \
    node_data->name = (void*)( \
        (uintptr_t)node_data + \
        sizeof(*node_data) + \
        buffer_alignment + \
        (uintptr_t)offsets.name);

    /* Add offsets calculated before to the base address of buffer */
#define X(upper, lower, type) ADD_OFFSET(lower)
    IK_ATTACHMENT_LIST
    IK_NODE_DATA_PROPERTIES_LIST
#undef X

    /* Index data offsets */
    ADD_OFFSET(base_idx)
    ADD_OFFSET(parent_idx)
    ADD_OFFSET(child_count)
    ADD_OFFSET(chain_depth)

#undef ADD_TO_SIZE
#undef ADD_OFFSET

    /* node_data is a refcounted type so it can be safely casted to
     * ik_refcount_t. This initializes the refcount */
    if ((status = ik_refcount_create(&node_data->refcount,
            (ik_deinit_func)deinit_node_data, 1)) != IK_OK)
        IK_FAIL(status, refcount_create_failed);

    /* Initialize fields that weren't covered by memset() */
    node_data->node_count = node_count;
    while (node_count--)
    {
        ik_quat_set_identity(node_data->transform[node_count].t.rotation.f);
        /* other fields are all memset to 0 */
    }

    return node_data;

    refcount_create_failed : FREE(node_data);
    alloc_node_data_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_data_create(struct ik_node_data_t** node_data)
{
    assert(node_data);

    *node_data = determine_zla_size_and_malloc(1);
    if (*node_data == NULL)
        return IK_ERR_OUT_OF_MEMORY;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_data_array_create(struct ik_node_data_t** node_data, uint32_t node_count)
{
    assert(node_data);

    *node_data = determine_zla_size_and_malloc(node_count);
    if (*node_data == NULL)
        return IK_ERR_OUT_OF_MEMORY;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_node_data_free(struct ik_node_data_t* node_data)
{
    IK_DECREF(node_data);
}

/* ------------------------------------------------------------------------- */
uint32_t
ik_node_data_find_highest_child_count(const struct ik_node_data_t* nd)
{
    size_t i;
    uint32_t max_children = 0;
    for (i = 0; i != nd->node_count; ++i)
    {
        if (max_children < nd->child_count[i])
            max_children = nd->child_count[i];
    }

    return max_children;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_data_view_create(struct ik_node_data_view_t** ndav,
                         struct ik_node_data_t* source,
                         uint32_t subbase_idx, uint32_t chain_begin_idx, uint32_t chain_end_idx)
{
    assert(ndav);
    assert(source);
    assert(chain_end_idx <= source->node_count);
    assert(chain_begin_idx <= chain_end_idx);
    assert(subbase_idx < chain_begin_idx);

    *ndav = MALLOC(sizeof **ndav);
    if (*ndav == NULL)
        return IK_ERR_OUT_OF_MEMORY;
    return ik_node_data_view_init(*ndav, source, subbase_idx, chain_begin_idx, chain_end_idx);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_data_view_init(struct ik_node_data_view_t* ndav,
                       struct ik_node_data_t* source,
                       uint32_t subbase_idx, uint32_t chain_begin_idx, uint32_t chain_end_idx)
{
    ndav->subbase_idx = subbase_idx;
    ndav->chain_begin_idx = chain_begin_idx;
    ndav->chain_end_idx = chain_end_idx;
    ndav->node_data = source;
    IK_INCREF(ndav->node_data);

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_node_data_view_deinit(struct ik_node_data_view_t* ndav)
{
    IK_DECREF(ndav->node_data);
}

/* ------------------------------------------------------------------------- */
void
ik_node_data_array_free(struct ik_node_data_view_t* ndav)
{
    ik_node_data_view_deinit(ndav);
    FREE(ndav);
}
