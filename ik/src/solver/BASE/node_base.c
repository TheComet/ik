#include "ik/node_base.h"
#include "ik/ik.h"
#include "ik/memory.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_node_base_create(uint32_t guid)
{
    struct ik_node_t* node = MALLOC(sizeof *node);
    if (node == NULL)
        return NULL;
    ik_node_base_construct(node, guid);

    return node;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_base_construct(struct ik_node_t* node, uint32_t guid)
{
    memset(node, 0, sizeof *node);
    bstv_construct(&node->children);
    node->v = &ik.internal.node_base;
    node->guid = guid;
    quat_set_identity(node->rotation.f);
    vec3_set_zero(node->rotation.f);

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static void
destroy_recursive(struct ik_node_t* node);
static void
destruct_recursive(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        destroy_recursive(child);
    NODE_END_EACH

    if (node->effector)
        node->effector->v->destroy(node->effector);
    if (node->constraint)
        node->constraint->v->destroy(node->constraint);

    bstv_clear_free(&node->children);
}
void
ik_node_base_destruct(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        destroy_recursive(child);
    NODE_END_EACH

    if (node->effector)
        node->effector->v->destroy(node->effector);
    if (node->constraint)
        node->constraint->v->destroy(node->constraint);

    node->v->unlink(node);
    bstv_clear_free(&node->children);
}

/* ------------------------------------------------------------------------- */
static void
destroy_recursive(struct ik_node_t* node)
{
    destruct_recursive(node);
    FREE(node);
}
void
ik_node_base_destroy(struct ik_node_t* node)
{
    if (ik.internal.callbacks->on_node_destroy != NULL)
        ik.internal.callbacks->on_node_destroy(node);
    node->v->destruct(node);
    FREE(node);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_base_add_child(struct ik_node_t* node, struct ik_node_t* child)
{
    ikret_t result;
    if ((result = bstv_insert(&node->children, child->guid, child)) != IK_OK)
        return result;
    child->parent = node;
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_node_base_unlink(struct ik_node_t* node)
{
    if (node->parent == NULL)
        return;

    bstv_erase(&node->parent->children, node->guid);
    node->parent = NULL;
}

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_node_base_find_child(const struct ik_node_t* node, uint32_t guid)
{
    struct ik_node_t* found = bstv_find(&node->children, guid);
    if (found != NULL)
        return found;

    if (node->guid == guid)
        return (struct ik_node_t*)node;

    NODE_FOR_EACH(node, child_guid, child)
        found = node->v->find_child(child, guid);
        if (found != NULL)
            return found;
    NODE_END_EACH

    return NULL;
}

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_node_base_duplicate(const struct ik_node_t* node, int copy_attachments)
{
    struct ik_node_t* new_node = node->v->create(node->guid);
    if (new_node == NULL)
        goto malloc_new_node_failed;

    /* Copy over transform data and other properties */
    new_node->v = node->v;
    new_node->rotation = node->rotation;
    new_node->position = node->position;
    new_node->initial_rotation = node->initial_rotation;
    new_node->initial_position = node->initial_position;
    new_node->stiffness = node->stiffness;
    new_node->rotation_weight = node->rotation_weight;
    new_node->dist_to_parent = node->dist_to_parent;
    new_node->user_data = node->user_data;

    if (copy_attachments)
    {
        if (node->effector != NULL)
        {
            const struct ik_effector_interface_t* ei = node->effector->v;
            struct ik_effector_t* new_effector = ei->create();
            if (new_effector == NULL)
                goto copy_child_node_failed;
            memcpy(new_effector, node->effector, sizeof *new_effector);
            ei->attach(new_effector, new_node);
        }
        if (node->constraint != NULL)
        {
            const struct ik_constraint_interface_t* ci = node->constraint->v;
            struct ik_constraint_t* new_constraint = ci->create(node->constraint->type);
            if (new_constraint == NULL)
                goto copy_child_node_failed;
            memcpy(new_constraint, node->constraint, sizeof *new_constraint);
            ci->attach(new_constraint, new_node);
        }
    }

    NODE_FOR_EACH(node, child_guid, child)
        struct ik_node_t* new_child_node = node->v->duplicate(child, copy_attachments);
        if (new_child_node == NULL)
            goto copy_child_node_failed;
        node->v->add_child(new_node, new_child_node);
    NODE_END_EACH

    return new_node;

    copy_child_node_failed  : node->v->destroy(new_node);
    malloc_new_node_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
static void
recursively_dump_dot(FILE* fp, struct ik_node_t* node)
{
    if (node->effector != NULL)
        fprintf(fp, "    %d [color=\"1.0 0.5 1.0\"];\n", node->guid);

    NODE_FOR_EACH(node, guid, child)
        fprintf(fp, "    %d -- %d;\n", node->guid, guid);
        recursively_dump_dot(fp, child);
    NODE_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_node_base_dump_to_dot(struct ik_node_t* node, const char* file_name)
{
    FILE* fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        ik.log.message("Failed to open file %s", file_name);
        return;
    }

    fprintf(fp, "graph graphname {\n");
    recursively_dump_dot(fp, node);
    fprintf(fp, "}\n");

    fclose(fp);
}
