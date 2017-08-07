#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

/* ------------------------------------------------------------------------- */
ik_node_t*
ik_node_create(uint32_t guid)
{
    ik_node_t* node = (ik_node_t*)MALLOC(sizeof *node);
    if (node == NULL)
        return NULL;

    ik_node_construct(node, guid);
    return node;
}

/* ------------------------------------------------------------------------- */
void
ik_node_construct(ik_node_t* node, uint32_t guid)
{
    memset(node, 0, sizeof *node);
    bstv_construct(&node->children);
    quat_set_identity(node->original_rotation.f);
    quat_set_identity(node->rotation.f);
    node->guid = guid;
}

/* ------------------------------------------------------------------------- */
static void
ik_node_destroy_recursive(ik_node_t* node);
static void
ik_node_destruct_recursive(ik_node_t* node)
{
    BSTV_FOR_EACH(&node->children, ik_node_t, guid, child)
        ik_node_destroy_recursive(child);
    BSTV_END_EACH

    if (node->effector)
        ik_effector_destroy(node->effector);
    if (node->constraint)
        ik_constraint_destroy(node->constraint);

    bstv_clear_free(&node->children);
}
void
ik_node_destruct(ik_node_t* node)
{
    BSTV_FOR_EACH(&node->children, ik_node_t, guid, child)
        ik_node_destroy_recursive(child);
    BSTV_END_EACH

    if (node->effector)
        ik_effector_destroy(node->effector);
    if (node->constraint)
        ik_constraint_destroy(node->constraint);

    ik_node_unlink(node);
    bstv_clear_free(&node->children);
}

/* ------------------------------------------------------------------------- */
static void
ik_node_destroy_recursive(ik_node_t* node)
{
    ik_node_destruct_recursive(node);
    FREE(node);
}
void
ik_node_destroy(ik_node_t* node)
{
    ik_node_destruct(node);
    FREE(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_add_child(ik_node_t* node, ik_node_t* child)
{
    child->parent = node;
    bstv_insert(&node->children, child->guid, child);
}

/* ------------------------------------------------------------------------- */
void
ik_node_unlink(ik_node_t* node)
{
    if (node->parent == NULL)
        return;

    bstv_erase(&node->parent->children, node->guid);
    node->parent = NULL;
}

/* ------------------------------------------------------------------------- */
ik_node_t*
ik_node_find_child(ik_node_t* node, uint32_t guid)
{
    ik_node_t* found = bstv_find(&node->children, guid);
    if (found != NULL)
        return found;

    if (node->guid == guid)
        return node;

    BSTV_FOR_EACH(&node->children, ik_node_t, child_guid, child)
        found = ik_node_find_child(child, guid);
        if (found != NULL)
            return found;
    BSTV_END_EACH

    return NULL;
}

/* ------------------------------------------------------------------------- */
void
ik_node_attach_effector(ik_node_t* node, ik_effector_t* effector)
{
    if (node->effector != NULL)
        ik_effector_destroy(node->effector);

    node->effector = effector;
}

/* ------------------------------------------------------------------------- */
void
ik_node_destroy_effector(ik_node_t* node)
{
    if (node->effector == NULL)
        return;
    ik_effector_destroy(node->effector);
    node->effector = NULL;
}

/* ------------------------------------------------------------------------- */
void
ik_node_attach_constraint(ik_node_t* node, ik_constraint_t* constraint)
{
    if (node->constraint != NULL)
        ik_constraint_destroy(node->constraint);

    node->constraint = constraint;
}

/* ------------------------------------------------------------------------- */
void
ik_node_destroy_constraint(ik_node_t* node)
{
    if (node->constraint == NULL)
        return;
    ik_constraint_destroy(node->constraint);
    node->constraint = NULL;
}

/* ------------------------------------------------------------------------- */
static void
recursively_dump_dot(FILE* fp, ik_node_t* node)
{
    if (node->effector != NULL)
        fprintf(fp, "    %d [color=\"1.0 0.5 1.0\"];\n", node->guid);

    BSTV_FOR_EACH(&node->children, ik_node_t, guid, child)
        fprintf(fp, "    %d -- %d;\n", node->guid, guid);
        recursively_dump_dot(fp, child);
    BSTV_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_node_dump_to_dot(ik_node_t* node, const char* file_name)
{
    FILE* fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        ik_log_message("Failed to open file %s", file_name);
        return;
    }

    fprintf(fp, "graph graphname {\n");
    recursively_dump_dot(fp, node);
    fprintf(fp, "}\n");

    fclose(fp);
}

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

    BSTV_FOR_EACH(&node->children, ik_node_t, guid, child)
        local_to_global_recursive(child, off_position, off_rotation, acc_pos, acc_rot);
    BSTV_END_EACH
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

    BSTV_FOR_EACH(&node->children, ik_node_t, guid, child)
        global_to_local_recursive(child, off_position, off_rotation, acc_pos, acc_rot);
    BSTV_END_EACH
}
static void
local_to_global_rotation_recursive(ik_node_t* node, uintptr_t off_rotation, quat_t acc_rot)
{
    /* Get rotation field in ik_node_t struct */
    quat_t* node_rotation = (quat_t*)((char*)node + off_rotation);

    quat_t rotation = *node_rotation;
    quat_mul_quat(node_rotation->f, acc_rot.f);
    quat_mul_quat(acc_rot.f, rotation.f);

    BSTV_FOR_EACH(&node->children, ik_node_t, guid, child)
        local_to_global_rotation_recursive(child, off_rotation, acc_rot);
    BSTV_END_EACH
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

    BSTV_FOR_EACH(&node->children, ik_node_t, guid, child)
        global_to_local_rotation_recursive(child, off_rotation, acc_rot);
    BSTV_END_EACH
}

typedef void (*transform_both_func)(ik_node_t*,uintptr_t,uintptr_t,vec3_t,quat_t);
typedef void (*transform_rot_func) (ik_node_t*,uintptr_t,quat_t);

/* ------------------------------------------------------------------------- */
static void
do_node_transform(ik_node_t* node, uint8_t flags,
                  transform_both_func transform_both,
                  transform_rot_func transform_rot)
{
    vec3_t acc_pos = {{0, 0, 0}};
    quat_t acc_rot = {{0, 0, 0, 1}};

    if (flags & NODE_ROTATIONS_ONLY)
    {
        if (flags & NODE_ACTIVE)
            transform_rot(node,
                          offsetof(ik_node_t, rotation),
                          acc_rot);
        if (flags & NODE_ORIGINAL)
            transform_rot(node,
                          offsetof(ik_node_t, original_rotation),
                          acc_rot);
    }
    else
    {
        if (flags & NODE_ACTIVE)
            transform_both(node,
                           offsetof(ik_node_t, position), offsetof(ik_node_t, rotation),
                           acc_pos, acc_rot);
        if (flags & NODE_ORIGINAL)
            transform_both(node,
                           offsetof(ik_node_t, original_position), offsetof(ik_node_t, original_rotation),
                           acc_pos, acc_rot);
    }
}

/* ------------------------------------------------------------------------- */
void
ik_node_local_to_global(ik_node_t* node, uint8_t flags)
{
    do_node_transform(node, flags, local_to_global_recursive, local_to_global_rotation_recursive);
}

/* ------------------------------------------------------------------------- */
void
ik_node_global_to_local(ik_node_t* node, uint8_t flags)
{
    do_node_transform(node, flags, global_to_local_recursive, global_to_local_rotation_recursive);
}
