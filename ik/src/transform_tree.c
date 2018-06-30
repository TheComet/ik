#include "ik/bstv.h"
#include "ik/quat.h"
#include "ik/transform.h"
#include "ik/vec3.h"
#include "ik/node.h"
#include <stddef.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
static void
local_to_global_recursive(struct ik_node_t* node, ikreal_t acc_rot_pos[7])
{
    vec3_t position;
    quat_t rotation;

    /* Unpack rotation (first 4 floats) and position (last 3 floats) from argument */
    ikreal_t* acc_rot = &acc_rot_pos[0];
    ikreal_t* acc_pos = &acc_rot_pos[4];

    vec3_rotate(node->position.f, acc_rot);
    position = node->position;
    vec3_add_vec3(node->position.f, acc_pos);
    vec3_add_vec3(acc_pos, position.f);

    rotation = node->rotation;
    quat_mul_quat(node->rotation.f, acc_rot);
    quat_mul_quat(acc_rot, rotation.f);

    NODE_FOR_EACH(node, guid, child)
        ikreal_t acc_rot_pos_child[7]; /* Have to copy due to tree structure */
        quat_set(&acc_rot_pos_child[0], acc_rot);
        vec3_set(&acc_rot_pos_child[4], acc_pos);
        local_to_global_recursive(child, acc_rot_pos_child);
    NODE_END_EACH
}
static void
global_to_local_recursive(struct ik_node_t* node, ikreal_t acc_rot_pos[7])
{
    quat_t inv_rot_acc;

    /* Unpack rotation (first 4 floats) and position (last 3 floats) from argument */
    ikreal_t* acc_rot = &acc_rot_pos[0];
    ikreal_t* acc_pos = &acc_rot_pos[4];

    quat_set(inv_rot_acc.f, acc_rot);
    quat_conj(inv_rot_acc.f);
    quat_mul_quat(node->rotation.f, inv_rot_acc.f);
    quat_mul_quat(acc_rot, node->rotation.f);

    vec3_sub_vec3(node->position.f, acc_pos);
    vec3_add_vec3(acc_pos, node->position.f);
    vec3_rotate(node->position.f, inv_rot_acc.f);

    NODE_FOR_EACH(node, guid, child)
        ikreal_t acc_rot_pos_child[7]; /* Have to copy due to tree structure */
        quat_set(&acc_rot_pos_child[0], acc_rot);
        vec3_set(&acc_rot_pos_child[4], acc_pos);
        global_to_local_recursive(child, acc_rot_pos_child);
    NODE_END_EACH
}
static void
local_to_global_rotation_recursive(struct ik_node_t* node, ikreal_t acc_rot[4])
{
    quat_t rotation = node->rotation;
    quat_mul_quat(node->rotation.f, acc_rot);
    quat_mul_quat(acc_rot, rotation.f);

    NODE_FOR_EACH(node, guid, child)
        ikreal_t acc_rot_child[4]; /* Have to copy due to tree structure */
        quat_set(acc_rot_child, acc_rot);
        local_to_global_rotation_recursive(child, acc_rot_child);
    NODE_END_EACH
}
static void
global_to_local_rotation_recursive(struct ik_node_t* node, ikreal_t acc_rot[4])
{
    quat_t inv_rot_acc;
    quat_set(inv_rot_acc.f, acc_rot);
    quat_conj(inv_rot_acc.f);
    quat_mul_quat(node->rotation.f, inv_rot_acc.f);
    quat_mul_quat(acc_rot, node->rotation.f);

    NODE_FOR_EACH(node, guid, child)
        ikreal_t acc_rot_child[4]; /* Have to copy due to tree structure */
        quat_set(acc_rot_child, acc_rot);
        global_to_local_rotation_recursive(child, acc_rot_child);
    NODE_END_EACH
}
static void
local_to_global_translation_recursive(struct ik_node_t* node, ikreal_t acc_rot_pos[7])
{
    vec3_t position;

    /* Unpack rotation (first 4 floats) and position (last 3 floats) from argument */
    ikreal_t* acc_rot = &acc_rot_pos[0];
    ikreal_t* acc_pos = &acc_rot_pos[4];

    vec3_rotate(node->position.f, acc_rot);
    position = node->position;
    vec3_add_vec3(node->position.f, acc_pos);
    vec3_add_vec3(acc_pos, position.f);

    quat_mul_quat(acc_rot, node->rotation.f);

    NODE_FOR_EACH(node, guid, child)
        ikreal_t acc_rot_pos_child[7]; /* Have to copy due to tree structure */
        quat_set(&acc_rot_pos_child[0], acc_rot);
        vec3_set(&acc_rot_pos_child[4], acc_pos);
        local_to_global_translation_recursive(child, acc_rot_pos_child);
    NODE_END_EACH
}
static void
global_to_local_translation_recursive(struct ik_node_t* node, ikreal_t acc_rot_pos[7])
{
    quat_t inv_rot_acc;

    /* Unpack rotation (first 4 floats) and position (last 3 floats) from argument */
    ikreal_t* acc_rot = &acc_rot_pos[0];
    ikreal_t* acc_pos = &acc_rot_pos[4];

    quat_set(inv_rot_acc.f, acc_rot);
    quat_conj(inv_rot_acc.f);

    quat_mul_quat(acc_rot, node->rotation.f);

    vec3_sub_vec3(node->position.f, acc_pos);
    vec3_add_vec3(acc_pos, node->position.f);
    vec3_rotate(node->position.f, inv_rot_acc.f);

    NODE_FOR_EACH(node, guid, child)
        ikreal_t acc_rot_pos_child[7]; /* Have to copy due to tree structure */
        quat_set(&acc_rot_pos_child[0], acc_rot);
        vec3_set(&acc_rot_pos_child[4], acc_pos);
        global_to_local_translation_recursive(child, acc_rot_pos_child);
    NODE_END_EACH
}

/* ------------------------------------------------------------------------- */
typedef void (*transform_func)(struct ik_node_t*, ikreal_t*);

static transform_func transform_table[8] = {
    global_to_local_recursive,
    local_to_global_recursive,
    global_to_local_rotation_recursive,
    local_to_global_rotation_recursive,
    global_to_local_translation_recursive,
    local_to_global_translation_recursive,
    global_to_local_recursive,
    local_to_global_recursive
};

/* ------------------------------------------------------------------------- */
void
ik_transform_tree(struct ik_node_t* node, uint8_t flags)
{
    ikreal_t base_transform[7];
    memcpy(base_transform, node->transform, sizeof(ikreal_t) * 7);

    (*transform_table[flags])(node, base_transform);
}
