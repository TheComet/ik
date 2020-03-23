#include "ik/algorithm.h"
#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/pole.h"
#include "ik/log.h"
#include "ik/node.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define GUID(n) (btree_key_t)((n)->user.guid)

/* ------------------------------------------------------------------------- */
static void
ik_node_deinit(struct ik_node* node)
{
    assert(node);

    /* Unref children */
    NODE_FOR_EACH(node, user_data, child)
        IK_DECREF(child);
    NODE_END_EACH

    /* unref attachments */
#define X(upper, lower) ik_node_destroy_##lower(node);
    IK_ATTACHMENT_LIST
#undef X

    /* deinit */
    btree_deinit(&node->children);
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_node_create(union ik_node_user_data user)
{
    struct ik_node* node = (struct ik_node*)
        ik_refcounted_alloc(sizeof *node, (ik_deinit_func)ik_node_deinit);
    if (node == NULL)
        goto alloc_node_failed;

    node->user = user;

    node->algorithm = NULL;
    node->constraint = NULL;
    node->effector = NULL;
    node->pole = NULL;

    ik_vec3_set_zero(node->trans.t.pos.f);
    ik_quat_set_identity(node->trans.t.rot.f);
    node->dist_to_parent = 0;
    node->rotation_weight = 0;
    node->mass = 0;

    node->parent = NULL;

    if (btree_init(&node->children, sizeof(struct ik_node*)) != BTREE_OK)
    {
        ik_log_out_of_memory("btree_init()");
        goto init_node_failed;
    }

    return node;

    init_node_failed  : ik_refcounted_free((struct ik_refcounted*)node);
    alloc_node_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_node_create_child(struct ik_node* parent, union ik_node_user_data user)
{
    struct ik_node* child;
    if ((child = ik_node_create(user)) == NULL)
        goto create_child_failed;
    if (ik_node_link(parent, child) != IK_OK)
        goto add_child_failed;

    return child;

    add_child_failed    : IK_DECREF(child);
    create_child_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
ikret
ik_node_link(struct ik_node* node, struct ik_node* child)
{
    assert(node);
    assert(child);
    assert(child != node);

    /* Searches the entire tree for the child guid -- disabled in release mode
     * for performance reasons */
#ifdef DEBUG
    {
        struct ik_node* found;
        struct ik_node* root = node;
        while (root->parent)
            root = root->parent;
        if (ik_node_find_child(&found, root, GUID(child)) == IK_OK)
            ik_log_warning("Child guid %d already exists in the tree! It will be inserted, but find_child() will only find one of the two.", GUID(child));
    }
#endif

    if (btree_insert_new(&node->children, GUID(child), &child) != BTREE_OK)
    {
        ik_log_printf(IK_ERROR, "Child guid %d already exists in this node's list of children! Node was not inserted into the tree.", GUID(child));
        return IK_ERR_DUPLICATE_NODE;
    }

    /* May already be part of a tree */
    if (child->parent)
        btree_erase(&child->parent->children, GUID(child));

    child->parent = node;
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_node_unlink(struct ik_node* node)
{
    assert(node);

    if (node->parent == NULL)
        return;

    btree_erase(&node->parent->children, GUID(node));
    node->parent = NULL;
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_node_find(struct ik_node* node, union ik_node_user_data user)
{
    /* O(log(n)) search of children */
    struct ik_node** found = btree_find(&node->children, user.guid);
    if (found != NULL)
        return *found;

    /* might be searching for this node? */
    if (GUID(node) == user.guid)
        return (struct ik_node*)node;

    NODE_FOR_EACH(node, child_guid, child)
        struct ik_node* found_child;
        if ((found_child = ik_node_find(child, user)) != NULL)
            return found_child;
    NODE_END_EACH

    return NULL;
}

/* ------------------------------------------------------------------------- */
#define X(upper, lower)                                                       \
    struct ik_##lower*                                                        \
    ik_node_create_##lower(struct ik_node* node) {                            \
        struct ik_##lower* lower = ik_##lower##_create();                     \
        if (lower == NULL)                                                    \
            return NULL;                                                      \
                                                                              \
        ik_node_attach_##lower(node, lower);                                  \
        return lower;                                                         \
    }                                                                         \
                                                                              \
    void                                                                      \
    ik_node_attach_##lower(struct ik_node* node, struct ik_##lower* lower) {  \
        if (lower->node)                                                      \
            ik_node_detach_##lower(lower->node);                              \
        ik_node_detach_##lower(node);                                         \
        node->lower = lower;                                                  \
        lower->node = node;                                                   \
    }                                                                         \
                                                                              \
    struct ik_##lower*                                                        \
    ik_node_detach_##lower(struct ik_node* node) {                            \
        struct ik_##lower* lower = node->lower;                               \
        node->lower = NULL;                                                   \
        if (lower)                                                            \
            lower->node = NULL;                                               \
        return lower;                                                         \
    }                                                                         \
                                                                              \
    void                                                                      \
    ik_node_destroy_##lower(struct ik_node* node) {                           \
        struct ik_##lower* lower = ik_node_detach_##lower(node);              \
        IK_XDECREF(lower);                                                    \
    }
IK_ATTACHMENT_LIST
#undef X
