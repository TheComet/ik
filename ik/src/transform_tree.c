#include "ik/bst_vector.h"
#include "ik/node.h"
#include "ik/quat.h"
#include "ik/transform.h"
#include "ik/vec3.h"
#include <stddef.h>

/* ------------------------------------------------------------------------- */
static void
local_to_global_recursive(ik_node_t* node,
                          uintptr_t off_position, uintptr_t off_rotation,
                          vec3_t acc_pos, quat_t acc_rot)
{
    vec3_t position;
    quat_t rotation;
    /* Get position and rotation fields in ik_node_t struct */
    vec3_t* node_position = (vec3_t*)((char*)node + off_position);
    quat_t* node_rotation = (quat_t*)((char*)node + off_rotation);

    quat_rotate_vec(node_position->f, acc_rot.f);
    position = *node_position;
    vec3_add_vec3(node_position->f, acc_pos.f);
    vec3_add_vec3(acc_pos.f, position.f);

    rotation = *node_rotation;
    quat_mul_quat(node_rotation->f, acc_rot.f);
    quat_mul_quat(acc_rot.f, rotation.f);

    NODE_FOR_EACH(node, guid, child)
        local_to_global_recursive(child, off_position, off_rotation, acc_pos, acc_rot);
    NODE_END_EACH
}
static void
global_to_local_recursive(ik_node_t* node,
                          uintptr_t off_position, uintptr_t off_rotation,
                          vec3_t acc_pos, quat_t acc_rot)
{
    quat_t inv_rotation = acc_rot;
    /* Get position and rotation fields in ik_node_t struct */
    vec3_t* node_position = (vec3_t*)((char*)node + off_position);
    quat_t* node_rotation = (quat_t*)((char*)node + off_rotation);

    quat_conj(inv_rotation.f);
    quat_mul_quat(node_rotation->f, inv_rotation.f);
    quat_mul_quat(acc_rot.f, node_rotation->f);

    vec3_sub_vec3(node_position->f, acc_pos.f);
    vec3_add_vec3(acc_pos.f, node_position->f);
    quat_rotate_vec(node_position->f, inv_rotation.f);

    NODE_FOR_EACH(node, guid, child)
        global_to_local_recursive(child, off_position, off_rotation, acc_pos, acc_rot);
    NODE_END_EACH
}
static void
local_to_global_rotation_recursive(ik_node_t* node, uintptr_t off_rotation, quat_t acc_rot)
{
    /* Get rotation field in ik_node_t struct */
    quat_t* node_rotation = (quat_t*)((char*)node + off_rotation);

    quat_t rotation = *node_rotation;
    quat_mul_quat(node_rotation->f, acc_rot.f);
    quat_mul_quat(acc_rot.f, rotation.f);

    NODE_FOR_EACH(node, guid, child)
        local_to_global_rotation_recursive(child, off_rotation, acc_rot);
    NODE_END_EACH
}
static void
global_to_local_rotation_recursive(ik_node_t* node, uintptr_t off_rotation, quat_t acc_rot)
{
    /* Get rotation field in ik_node_t struct */
    quat_t* node_rotation = (quat_t*)((char*)node + off_rotation);

    quat_t inv_rotation = acc_rot;
    quat_conj(inv_rotation.f);
    quat_mul_quat(node_rotation->f, inv_rotation.f);
    quat_mul_quat(acc_rot.f, node_rotation->f);

    NODE_FOR_EACH(node, guid, child)
        global_to_local_rotation_recursive(child, off_rotation, acc_rot);
    NODE_END_EACH
}

typedef void (*transform_both_func)(ik_node_t*,uintptr_t,uintptr_t,vec3_t,quat_t);
typedef void (*transform_rot_func) (ik_node_t*,uintptr_t,quat_t);

/* ------------------------------------------------------------------------- */
static void
do_transform(ik_node_t* node, uint8_t flags,
             transform_both_func transform_both,
             transform_rot_func transform_rot)
{
    vec3_t acc_pos = {{0, 0, 0}};
    quat_t acc_rot = {{0, 0, 0, 1}};

    if (flags & TRANSFORM_ROTATIONS_ONLY)
    {
        if (flags & TRANSFORM_ACTIVE)
            transform_rot(node,
                          offsetof(ik_node_t, rotation),
                          acc_rot);
        if (flags & TRANSFORM_ORIGINAL)
            transform_rot(node,
                          offsetof(ik_node_t, original_rotation),
                          acc_rot);
    }
    else
    {
        if (flags & TRANSFORM_ACTIVE)
            transform_both(node,
                           offsetof(ik_node_t, position), offsetof(ik_node_t, rotation),
                           acc_pos, acc_rot);
        if (flags & TRANSFORM_ORIGINAL)
            transform_both(node,
                           offsetof(ik_node_t, original_position), offsetof(ik_node_t, original_rotation),
                           acc_pos, acc_rot);
    }
}

/* ------------------------------------------------------------------------- */
void
ik_tree_local_to_global(ik_node_t* node, uint8_t flags)
{
    do_transform(node, flags, local_to_global_recursive, local_to_global_rotation_recursive);
}

/* ------------------------------------------------------------------------- */
void
ik_tree_global_to_local(ik_node_t* node, uint8_t flags)
{
    do_transform(node, flags, global_to_local_recursive, global_to_local_rotation_recursive);
}
