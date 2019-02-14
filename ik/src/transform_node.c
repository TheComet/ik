#include "ik/btree.h"
#include "ik/node.h"
#include "ik/quat.h"
#include "ik/transform.h"
#include "ik/vec3.h"
#include <stddef.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
static void
local_to_global_recursive(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        ik_vec3_nrotate(child->position.f, node->rotation.f);
        ik_vec3_add_vec3(child->position.f, node->position.f);
        ik_quat_mul_quat(child->rotation.f, node->rotation.f);
        local_to_global_recursive(child);
    NODE_END_EACH
}
static void
global_to_local_recursive(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        global_to_local_recursive(child);
        ik_vec3_sub_vec3(child->position.f, node->position.f);
        ik_vec3_rotate(child->position.f, node->rotation.f);
        ik_quat_nmul_quat(child->rotation.f, node->rotation.f);
    NODE_END_EACH
}
static void
local_to_global_rotation_recursive(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        ik_quat_mul_quat(child->rotation.f, node->rotation.f);
        local_to_global_rotation_recursive(child);
    NODE_END_EACH
}
static void
global_to_local_rotation_recursive(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        global_to_local_rotation_recursive(child);
        ik_quat_nmul_quat(child->rotation.f, node->rotation.f);
    NODE_END_EACH
}
static void
local_to_global_translation_recursive(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        ik_vec3_nrotate(child->position.f, node->rotation.f);
        ik_vec3_add_vec3(child->position.f, node->position.f);
        local_to_global_translation_recursive(child);
    NODE_END_EACH
}
static void
global_to_local_translation_recursive(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        global_to_local_translation_recursive(child);
        ik_vec3_sub_vec3(child->position.f, node->position.f);
        ik_vec3_rotate(child->position.f, node->rotation.f);
    NODE_END_EACH
}

/* ------------------------------------------------------------------------- */
static void (*transform_table[8])(struct ik_node_t*) = {
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
ik_transform_node(struct ik_node_t* node, uint8_t flags)
{
    (*transform_table[flags])(node);
}
