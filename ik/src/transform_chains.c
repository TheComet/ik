#include "ik/bstv.h"
#include "ik/quat_static.h"
#include "ik/transform_static.h"
#include "ik/vec3_static.h"
#include "ik/chain.h"
#include "ik/node.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>

/*
 * In all of the following algorithms, we iterate the nodes in a chain starting
 * with the node *after* the base node. This is because the base node is shared
 * by all chains in the list. If this current chain has children, then
 * transforming our tip is effectively transforming the base node of all of our
 * children. The only node left untransformed will be the root node, which
 * doesn't have to be transformed anyway, because its not relative to anything.
 */

/* ------------------------------------------------------------------------- */
static void
local_to_global_rotation_recursive(struct chain_t* chain, ikreal_t acc_rot[4])
{
    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        struct ik_node_t* node = chain_get_node(chain, idx);

        struct ik_quat_t rotation = node->rotation;
        ik_quat_static_mul_quat(node->rotation.f, acc_rot);
        ik_quat_static_mul_quat(acc_rot, rotation.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        ikreal_t acc_rot_child[4]; /* Have to copy due to tree structure */
        ik_quat_static_set(acc_rot_child, acc_rot);
        local_to_global_rotation_recursive(child, acc_rot_child);
    CHAIN_END_EACH
}
static void
global_to_local_rotation_recursive(struct chain_t* chain, ikreal_t acc_rot[4])
{
    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        struct ik_node_t* node = chain_get_node(chain, idx);

        struct ik_quat_t inv_rot_acc;
        ik_quat_static_set(inv_rot_acc.f, acc_rot);
        ik_quat_static_conj(inv_rot_acc.f);
        ik_quat_static_mul_quat(node->rotation.f, inv_rot_acc.f);
        ik_quat_static_mul_quat(acc_rot, node->rotation.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        ikreal_t acc_rot_child[4]; /* Have to copy due to tree structure */
        ik_quat_static_set(acc_rot_child, acc_rot);
        global_to_local_rotation_recursive(child, acc_rot_child);
    CHAIN_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
local_to_global_translation_recursive(struct chain_t* chain, ikreal_t acc_rot_pos[7])
{
    struct ik_vec3_t position;

    /* Unpack rotation (first 4 floats) and position (last 3 floats) from argument */
    ikreal_t* acc_rot = &acc_rot_pos[0];
    ikreal_t* acc_pos = &acc_rot_pos[4];

    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        struct ik_node_t* node = chain_get_node(chain, idx);

        ik_vec3_static_rotate(node->position.f, acc_rot);
        position = node->position;
        ik_vec3_static_add_vec3(node->position.f, acc_pos);
        ik_vec3_static_add_vec3(acc_pos, position.f);

        ik_quat_static_mul_quat(acc_rot, node->rotation.f);
    }

    /* Recurse into child chains */
    CHAIN_FOR_EACH_CHILD(chain, child)
        ikreal_t acc_rot_pos_child[7]; /* Have to copy due to tree structure */
        ik_quat_static_set(&acc_rot_pos_child[0], acc_rot);
        ik_vec3_static_set(&acc_rot_pos_child[4], acc_pos);
        local_to_global_translation_recursive(child, acc_rot_pos_child);
    CHAIN_END_EACH
}
static void
global_to_local_translation_recursive(struct chain_t* chain, ikreal_t acc_rot_pos[7])
{
    /* Unpack rotation (first 4 floats) and position (last 3 floats) from argument */
    ikreal_t* acc_rot = &acc_rot_pos[0];
    ikreal_t* acc_pos = &acc_rot_pos[4];

    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        struct ik_node_t* node = chain_get_node(chain, idx);

        struct ik_quat_t inv_rot_acc;
        ik_quat_static_set(inv_rot_acc.f, acc_rot);
        ik_quat_static_conj(inv_rot_acc.f);

        ik_quat_static_mul_quat(acc_rot, node->rotation.f);

        ik_vec3_static_sub_vec3(node->position.f, acc_pos);
        ik_vec3_static_add_vec3(acc_pos, node->position.f);
        ik_vec3_static_rotate(node->position.f, inv_rot_acc.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        ikreal_t acc_rot_pos_child[7]; /* Have to copy due to tree structure */
        ik_quat_static_set(&acc_rot_pos_child[0], acc_rot);
        ik_vec3_static_set(&acc_rot_pos_child[4], acc_pos);
        global_to_local_translation_recursive(child, acc_rot_pos_child);
    CHAIN_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
local_to_global_recursive(struct chain_t* chain, ikreal_t acc_rot_pos[7])
{
    struct ik_vec3_t position;
    struct ik_quat_t rotation;

    /* Unpack rotation (first 4 floats) and position (last 3 floats) from argument */
    ikreal_t* acc_rot = &acc_rot_pos[0];
    ikreal_t* acc_pos = &acc_rot_pos[4];

    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        struct ik_node_t* node = chain_get_node(chain, idx);

        ik_vec3_static_rotate(node->position.f, acc_rot);
        position = node->position;
        ik_vec3_static_add_vec3(node->position.f, acc_pos);
        ik_vec3_static_add_vec3(acc_pos, position.f);

        rotation = node->rotation;
        ik_quat_static_mul_quat(node->rotation.f, acc_rot);
        ik_quat_static_mul_quat(acc_rot, rotation.f);
    }

    /* Recurse into child chains */
    CHAIN_FOR_EACH_CHILD(chain, child)
        ikreal_t acc_rot_pos_child[7]; /* Have to copy due to tree structure */
        ik_quat_static_set(&acc_rot_pos_child[0], acc_rot);
        ik_vec3_static_set(&acc_rot_pos_child[4], acc_pos);
        local_to_global_recursive(child, acc_rot_pos_child);
    CHAIN_END_EACH
}
static void
global_to_local_recursive(struct chain_t* chain, ikreal_t acc_rot_pos[7])
{
    /* Unpack rotation (first 4 floats) and position (last 3 floats) from argument */
    ikreal_t* acc_rot = &acc_rot_pos[0];
    ikreal_t* acc_pos = &acc_rot_pos[4];

    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        struct ik_node_t* node = chain_get_node(chain, idx);

        struct ik_quat_t inv_rot_acc;
        ik_quat_static_set(inv_rot_acc.f, acc_rot);
        ik_quat_static_conj(inv_rot_acc.f);
        ik_quat_static_mul_quat(node->rotation.f, inv_rot_acc.f);
        ik_quat_static_mul_quat(acc_rot, node->rotation.f);

        ik_vec3_static_sub_vec3(node->position.f, acc_pos);
        ik_vec3_static_add_vec3(acc_pos, node->position.f);
        ik_vec3_static_rotate(node->position.f, inv_rot_acc.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        ikreal_t acc_rot_pos_child[7]; /* Have to copy due to tree structure */
        ik_quat_static_set(&acc_rot_pos_child[0], acc_rot);
        ik_vec3_static_set(&acc_rot_pos_child[4], acc_pos);
        global_to_local_recursive(child, acc_rot_pos_child);
    CHAIN_END_EACH
}

/* ------------------------------------------------------------------------- */
typedef void (*transform_func)(struct chain_t*, ikreal_t*);

static transform_func transform_table[8] = {
    global_to_local_recursive,
    local_to_global_recursive,
    global_to_local_rotation_recursive,
    local_to_global_rotation_recursive,
    global_to_local_translation_recursive,
    local_to_global_translation_recursive,
    global_to_local_recursive,
    local_to_global_recursive,
};

/* ------------------------------------------------------------------------- */
void
ik_transform_static_chain_list(const struct vector_t* chain_list, uint8_t flags)
{
    VECTOR_FOR_EACH(chain_list, struct chain_t, chain)
        ik_transform_static_chain(chain, flags);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_transform_static_chain(struct chain_t* chain, uint8_t flags)
{
    ikreal_t base_transform[7];

    assert(chain_length(chain) >= 2);
    memcpy(base_transform, chain_get_base_node(chain)->transform, sizeof(ikreal_t) * 7);

    (*transform_table[flags])(chain, base_transform);
}
