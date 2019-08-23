#ifndef IK_NODE_DATA_H
#define IK_NODE_DATA_H

#include "cstructures/vector.h"
#include "ik/config.h"
#include "ik/attachment.h"
#include "ik/refcount.h"
#include "ik/transform.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_BEGIN

/* Forward declaration of all attachment types */
#define X(upper, lower, type) type;
    IK_ATTACHMENT_LIST
#undef X

/*
 * X macro list of all user accessible properties that are part of the node
 * data struct. This is an X macro because it makes calculating the required
 * buffer size easier.
 */
#define IK_NODE_DATA_PROPERTIES_LIST                                          \
    X(USER_DATA,        user_data,       void*)                               \
    X(TRANSFORM,        transform,       union ik_transform_t)                \
    X(DIST_TO_PARENT,   dist_to_parent,  ikreal_t)                            \
    X(ROTATION_WEIGHT,  rotation_weight, ikreal_t)                            \
    X(MASS,             mass,            ikreal_t)

/*!
 * @brief An optimizable structure for holding node data such as 3D transform,
 * effectors, constraints, or other node data.
 *
 * Every ik_node_t object points to an instance of ik_node_data_t (let's call
 * this "nd") where it can store its data. Initially, every node will point
 * to a different node data structure allocated at different memory locations.
 *
 *     ______________            ______________            ______________
 *    |     node     |  parent  |     node     |  parent  |     node     |
 *    | data_index=0 |--------->| data_index=0 |--------->| data_index=0 |
 *    |______________|          |______________|          |______________|
 *           |                         |                         |
 *      _____v_____               _____v_____               _____v_____
 *     | node data |             | node data |             | node data |
 *     |___________|             |___________|             |___________|
 *           |                         |                         |
 *       ____v_____                ____v_____                ____v_____
 *      | refcount |              | refcount |              | refcount |
 *      |__________|              |__________|              |__________|
 *
 *
 * The actual data is stored contiguously in nd->buffer, which is a block of
 * memory allocated at the end of the ik_node_data_t structure. There structure
 * has pointer members that point into this buffer for convenience.
 *
 * struct ik_node_data_t {
 *                / transform*  ----------
 *       params  |  dist_to_parent*  -----|------
 *                \ ...                ___v__ ___v____ _____
 *                  buffer[]          | t[0] | dtp[0] | ... |
 * }
 *
 * This is not very cache coherent when the solver iterates the tree hundreds
 * of times. Before solving, the tree data structure can be "optimized" such
 * that all node data is collected and merged into a single node data instance
 * holding all of the parameters in a contiguous array.
 *
 *      ______________            ______________            ______________
 *    |     node     |  parent  |     node     |  parent  |     node     |
 *    | data_index=2 |--------->| data_index=1 |--------->| data_index=0 |
 *    |______________|          |______________|          |______________|
 *                  \                   |                  /
 *                   \ _____________    |    _____________/
 *                                  \   |   /
 *                                 __v__v__v__
 *                                | node data |
 *                                |___________|
 *                                      |
 *                                  ____v_____
 *                                 | refcount |
 *                                 |__________|
 *
 * Notice how each node now has a different value for data_index. This is so
 * it knows at what offset its node data can be found in the single node data
 * instance.
 *
 * The order in which the data is placed into the buffer is pre-order, which
 * means if you iterate over the node data buffer, you will be iterating over
 * the nodes in pre-order.
 *
 * The buffer is organized into chunks. Each pointer member in ik_node_data_t
 * points to the beginning of a chunk.
 *
 * struct ik_node_data_t {
 *                / transform*  ----------
 *        params |  dist_to_parent*  -----|---------------------
 *                \ ...                ___v__ ______ ______ ____v___ ________ ________ _____
 *                  buffer[]          | t[0] | t[1] | t[2] | dtp[0] | dtp[1] | dtp[2] | ... |
 * }
 *
 * For example, nd->transform[2] would retrieve the transform of the third node
 * in the buffer. Nodes access their transforms with n->data->transform[n->data_index].
 */
struct ik_node_data_t
{
    IK_REFCOUNT_HEAD

#define X(upper, lower, type) type** lower;
    IK_ATTACHMENT_LIST
#undef X

#define X(upper, lower, type) type* lower;
    IK_NODE_DATA_PROPERTIES_LIST
#undef X

    /*!
     *
     */
    uint32_t* base_idx;
    uint32_t* parent_idx;
    uint32_t* child_count;
    uint32_t* chain_depth;
    uint32_t* commands;

    /*!
     * The number of nodes that are contiguously allocated for each field. In
     * the case of a normal ik_node_t, this value will be 1. If a tree of nodes
     * is flattened, this will be greater than 1.
     */
    uint32_t node_count;
};

enum commands_e
{
    CMD_LOAD_EFFECTOR = 0x01
};

IK_PRIVATE_API IKRET
ik_node_data_create(struct ik_node_data_t** node_data);

IK_PRIVATE_API IKRET
ik_node_data_array_create(struct ik_node_data_t** node_data, uint32_t node_count);

IK_PRIVATE_API void
ik_node_data_free(struct ik_node_data_t* node_data);

IK_PRIVATE_API uint32_t
ik_node_data_find_highest_child_count(const struct ik_node_data_t* nd);

struct ik_node_data_view_t
{
    struct ik_node_data_t* node_data;
    uint32_t subbase_idx;
    uint32_t chain_begin_idx;
    uint32_t chain_end_idx;
};

IK_PRIVATE_API IKRET
ik_node_data_view_create(struct ik_node_data_view_t** ndav,
                         struct ik_node_data_t* source,
                         uint32_t subbase_idx, uint32_t chain_begin_idx, uint32_t chain_end_idx);

IK_PRIVATE_API IKRET
ik_node_data_view_init(struct ik_node_data_view_t* ndav,
                       struct ik_node_data_t* source,
                       uint32_t subbase_idx, uint32_t chain_begin_idx, uint32_t chain_end_idx);

IK_PRIVATE_API void
ik_node_data_view_deinit(struct ik_node_data_view_t* ndav);

IK_PRIVATE_API void
ik_node_data_array_free(struct ik_node_data_view_t* ndav);

#define IK_NDV_AT(ndv, member, idx) ( \
        (idx) == 0 ? \
            (ndv)->node_data->member[(ndv)->subbase_idx] : \
            (ndv)->node_data->member[(ndv)->chain_begin_idx + (idx)] \
        )

#define IK_NDV_FOR(ndv, idx) { \
        int idx; \
        for (idx = (ndv)->subbase_idx; \
             idx != (int)(ndv)->chain_end_idx; \
             idx = (int)(ndv)->subbase_idx ? (int)(ndv)->chain_begin_idx : idx + 1) {

#define IK_NDV_FOR_R(ndv, idx) { \
        int idx; \
        for (idx = (ndv)->chain_end_idx - 1; \
             idx != (int)(ndv)->subbase_idx - 1; \
             idx = (int)(ndv)->chain_begin_idx ? (int)(ndv)->subbase_idx : idx - 1) {

#define IK_NDV_END }}

C_END

#endif /* IK_NODE_DATA_H */
