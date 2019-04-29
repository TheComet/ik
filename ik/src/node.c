#include "ik/callbacks.h"
#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/node_data.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/pole.h"
#include "ik/quat.h"
#include "ik/solver.h"
#include "ik/vec3.h"
#include "cstructures/memory.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define IK_FAIL(errcode, label) do { \
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

    if ((status = ik_node_init(*node, user_data)) != IK_OK)
    {
        FREE(*node);
        return status;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_init(struct ik_node_t* node, const void* user_data)
{
    ikret_t status;
    assert(node);

    if ((status = ik_node_data_create(&node->d)) != IK_OK)
        IK_FAIL(status, create_node_data_failed);

    if (btree_init(&node->children, sizeof(struct ik_node_t*)) != BTREE_OK)
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, btree_init_failed);

    node->parent = NULL;
    node->data_index = 0;
    IK_NODE_USER_DATA(node) = user_data;

    return IK_OK;

    btree_init_failed       : ik_node_data_free(node->d);
    create_node_data_failed : return status;
}

/* ------------------------------------------------------------------------- */
static void deinit_data(struct ik_node_t* node)
{
    btree_deinit(&node->children);
    IK_DECREF(node->d);
}
static void
free_recursive(struct ik_node_t* node);
static void
deinit_recursive(struct ik_node_t* node)
{
    NODE_FOR_EACH(node, guid, child)
        free_recursive(child);
    NODE_END_EACH

    deinit_data(node);
}
static void
free_recursive(struct ik_node_t* node)
{
    deinit_recursive(node);
    FREE(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_deinit_recursive(struct ik_node_t* node)
{
    assert(node);

    NODE_FOR_EACH(node, guid, child)
        free_recursive(child);
    NODE_END_EACH

    ik_node_deinit(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_free_recursive(struct ik_node_t* node)
{
    ik_node_deinit_recursive(node);
    FREE(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_deinit(struct ik_node_t* node)
{
    assert(node);

    /*
     * This is the top-most node being deinited and therefore might have a
     * parent node. Unlinking makes sure the parent node has nothing more to
     * do with us
     */
    ik_node_unlink(node);

    /* Now safely deinit */
    deinit_data(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_free(struct ik_node_t* node)
{
    ik_node_deinit(node);
    FREE(node);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_link(struct ik_node_t* node, struct ik_node_t* child)
{
    uintptr_t child_guid = (uintptr_t)IK_NODE_USER_DATA(child);
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
        if (ik_node_find_child(&found, root, child_guid) == IK_OK)
            ik_log_warning("Child guid %d already exists in the tree! It will be inserted, but find_child() will only find one of the two.", child_guid);
    }
#endif

    if (btree_insert_new(&node->children, child_guid, &child) != BTREE_OK)
    {
        ik_log_error("Child guid %d already exists in this node's list of children! Node was not inserted into the tree.", child_guid);
        return IK_ERR_DUPLICATE_NODE;
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

    add_child_failed    : ik_node_free(*child);
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
               (uintptr_t)IK_NODE_USER_DATA(node));
    node->parent = NULL;
}

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_node_find(struct ik_node_t* node,
             const void* user_data)
{
    /* O(log(n)) search of children */
    struct ik_node_t** found = btree_find(&node->children, (uintptr_t)user_data);
    if (found != NULL)
        return *found;

    /* might be searching for this node? */
    if (IK_NODE_USER_DATA(node) == user_data)
        return (struct ik_node_t*)node;

    NODE_FOR_EACH(node, child_guid, child)
        struct ik_node_t* found_child;
        if ((found_child = ik_node_find(child, user_data)) != NULL)
            return found_child;
    NODE_END_EACH

    return NULL;
}

/* ------------------------------------------------------------------------- */
#define X(upper, lower, type) \
        ikret_t \
        ik_node_create_##lower(struct ik_##lower##_t** a, struct ik_node_t* node) \
        { \
            ikret_t status; \
            if ((status = ik_##lower##_create(a)) != IK_OK) \
                return status; \
            \
            if ((status = ik_node_attach_##lower(node, *a)) != IK_OK) \
            { \
                IK_DECREF(*a); \
                return status; \
            } \
            \
            return IK_OK; \
        } \
        \
        ikret_t \
        ik_node_attach_##lower(struct ik_node_t* node, struct ik_##lower##_t* a) \
        {\
            if (*node->d->lower != NULL) \
                return IK_ERR_ALREADY_HAS_ATTACHMENT; \
            \
            *node->d->lower = a; \
            return IK_OK; \
        } \
        \
        void \
        ik_node_release_##lower(struct ik_node_t* node) \
        { \
            IK_XDECREF(ik_node_take_##lower(node)); \
        } \
        \
        struct ik_##lower##_t* \
        ik_node_take_##lower(struct ik_node_t* node) \
        { \
            struct ik_##lower##_t* a; \
            \
            a = *node->d->lower; \
            *node->d->lower = NULL; \
            return a; \
        } \
        \
        struct ik_##lower##_t* \
        ik_node_get_##lower(const struct ik_node_t* node) \
        { \
            return *node->d->lower; \
        }
    IK_ATTACHMENT_LIST
#undef X
