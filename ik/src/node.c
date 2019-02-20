#include "ik/callbacks.h"
#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/node_data.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/pole.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define FAIL(errcode, label) do { \
        status = errcode; goto label; \
    } while (0)

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_create(struct ik_node_t** node,
               const void* user_data)
{
    ikret_t status;
    assert(node);

    *node = MALLOC(sizeof **node);
    if (*node == NULL)
    {
        ik_log_fatal("Failed to allocate node: Ran out of memory");
        return IK_ERR_OUT_OF_MEMORY;
    }

    if ((status = ik_node_construct(*node, user_data)) != IK_OK)
    {
        FREE(*node);
        return status;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_construct(struct ik_node_t* node, const void* user_data)
{
    ikret_t status;
    assert(node);

    if ((status = ik_node_data_create(&node->node_data, user_data)) != IK_OK)
        return status;

    node->parent = NULL;
    btree_construct(&node->children);

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static void destruct_data(struct ik_node_t* node)
{
    btree_clear_free(&node->children);
    IK_DECREF(node->node_data);
}
static void
destroy_recursive(struct ik_node_t* node);
static void
destruct_recursive(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        destroy_recursive(child);
    NODE_END_EACH

    destruct_data(node);
}
static void
destroy_recursive(struct ik_node_t* node)
{
    destruct_recursive(node);
    FREE(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_destruct_recursive(struct ik_node_t* node)
{
    assert(node);

    NODE_FOR_EACH(node, guid, child)
        destroy_recursive(child);
    NODE_END_EACH

    ik_node_destruct(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_destroy_recursive(struct ik_node_t* node)
{
    ik_node_destruct_recursive(node);
    FREE(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_destruct(struct ik_node_t* node)
{
    assert(node);

    /*
     * This is the top-most node being destructed and therefore might have a
     * parent node. Unlinking makes sure the parent node has nothing more to
     * do with us
     */
    ik_node_unlink(node);

    /* Now safely destruct */
    destruct_data(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_destroy(struct ik_node_t* node)
{
    ik_node_destruct(node);
    FREE(node);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_link(struct ik_node_t* node, struct ik_node_t* child)
{
    ikret_t result;
    uintptr_t child_guid = (uintptr_t)child->node_data->user_data;
    assert(node);
    assert(child);
    assert(child != node);

    /* May already be part of a tree */
    ik_node_unlink(child);

    /* Searches the entire tree for the child guid -- disabled in release mode
     * for performance reasons */
#ifdef DEBUG
    {
        struct ik_node_t* found;
        struct ik_node_t* root = node;
        while (root->parent)
            root = root->parent;
        if (ik_node_find_child(&found, root, child->node_data->user_data) == IK_OK)
            ik_log_warning("Child guid %d already exists in the tree! It will be inserted, but find_child() will only find one of the two.", child_guid);
    }
#endif

    if ((result = btree_insert(&node->children, child_guid, child)) != IK_OK)
    {
        ik_log_error("Child guid %d already exists in this node's list of children! Node was not inserted into the tree.", child_guid);
        return result;
    }

    child->parent = node;
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_create_child(struct ik_node_t** child,
                     struct ik_node_t* parent,
                     const void* user_data)
{
    ikret_t status;
    if ((status = ik_node_create(child, user_data)) != IK_OK)
        goto create_child_failed;
    if ((status = ik_node_link(parent, *child)) != IK_OK)
        goto add_child_failed;

    return IK_OK;

    add_child_failed    : ik_node_destroy(*child);
    create_child_failed : return status;
}

/* ------------------------------------------------------------------------- */
void
ik_node_unlink(struct ik_node_t* node)
{
    assert(node);

    if (node->parent == NULL)
        return;

    btree_erase(&node->parent->children,
               (uintptr_t)node->node_data->user_data);
    node->parent = NULL;
}

/* ------------------------------------------------------------------------- */
vector_size_t
ik_node_child_count(const struct ik_node_t* node)
{
    return btree_count(&node->children);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_find_child(struct ik_node_t** found,
                   const struct ik_node_t* node,
                   const void* user_data)
{
    *found = btree_find(&node->children, (uintptr_t)user_data);
    if (*found != NULL)
        return IK_OK;

    if (ik_node_get_user_data(node) == user_data)
    {
        *found = (struct ik_node_t*)node;
        return IK_OK;
    }

    NODE_FOR_EACH(node, child_guid, child)
        if (ik_node_find_child(found, child, user_data) == IK_OK)
            return IK_OK;
    NODE_END_EACH

    return IK_NODE_NOT_FOUND;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_create_effetor(struct ik_effector_t** eff, struct ik_node_t* node)
{
    ikret_t status;

    if (node->node_data->effector != NULL)
        return IK_ERR_ALREADY_HAS_ATTACHMENT;

    if ((status = ik_effector_create(&node->node_data->effector)) != IK_OK)
        return status;

    *eff = node->node_data->effector;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_node_destroy_effector(struct ik_node_t* node)
{
    if (node->node_data->effector == NULL)
        return;

    IK_DECREF(node->node_data->effector);
    node->node_data->effector = NULL;
}

/* ------------------------------------------------------------------------- */
void
ik_node_set_position(struct ik_node_t* node, const ikreal_t position[3])
{
    ik_vec3_copy(node->node_data->transform.t.position.f, position);
}

/* ------------------------------------------------------------------------- */
void
ik_node_set_rotation(struct ik_node_t* node, const ikreal_t rotation[4])
{
    ik_quat_copy(node->node_data->transform.t.rotation.f, rotation);
}

/* ------------------------------------------------------------------------- */
void
ik_node_set_rotation_weight(struct ik_node_t* node, ikreal_t rotation_weight)
{
    if (rotation_weight > 1.0) rotation_weight = 1.0;
    if (rotation_weight < 0.0) rotation_weight = 0.0;
    node->node_data->rotation_weight = rotation_weight;
}

/* ------------------------------------------------------------------------- */
void
ik_node_set_mass(struct ik_node_t* node, ikreal_t mass)
{
    if (mass < 0.0) mass = 0.0;
    node->node_data->mass = mass;
}

/* ------------------------------------------------------------------------- */

const ikreal_t*
ik_node_get_position(const struct ik_node_t* node)
{
    return node->node_data->transform.t.position.f;
}

/* ------------------------------------------------------------------------- */
const ikreal_t*
ik_node_get_rotation(const struct ik_node_t* node)
{
    return node->node_data->transform.t.rotation.f;
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_node_get_rotation_weight(const struct ik_node_t* node)
{
    return node->node_data->rotation_weight;
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_node_get_mass(const struct ik_node_t* node)
{
    return node->node_data->mass;
}

/* ------------------------------------------------------------------------- */

ikreal_t
ik_node_get_distance_to_parent(const struct ik_node_t* node)
{
    return node->node_data->dist_to_parent;
}

/* ------------------------------------------------------------------------- */
const void*
ik_node_get_user_data(const struct ik_node_t* node)
{
    return node->node_data->user_data;
}

/* ------------------------------------------------------------------------- */
uintptr_t
ik_node_get_uid(const struct ik_node_t* node)
{
    return (uintptr_t)ik_node_get_user_data(node);
}

/* ------------------------------------------------------------------------- */
const struct ik_effector_t*
ik_node_get_effector(const struct ik_node_t* node)
{
    return node->node_data->effector;
}

/* ------------------------------------------------------------------------- */
const struct ik_constraint_t*
ik_node_get_constraint(const struct ik_node_t* node)
{
    return node->node_data->constraint;
}

/* ------------------------------------------------------------------------- */
const struct ik_pole_t*
ik_node_get_pole(const struct ik_node_t* node)
{
    return node->node_data->pole;
}
