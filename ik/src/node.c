#include "ik/algorithm.h"
#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/pole.h"
#include "ik/quat.inl"
#include "ik/vec3.inl"
#include <string.h>
#include <assert.h>

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
#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower) ik_node_detach_##lower(node);
    IK_ATTACHMENT_LIST
#undef X
#undef X1

    /* deinit */
    btree_deinit(&node->children);
}

/* ------------------------------------------------------------------------- */
static void
ik_node_destroy(struct ik_node* node)
{
    ik_node_deinit(node);
    ik_refcounted_free((struct ik_refcounted*)node);
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_node_create(union ik_node_user_data user)
{
    struct ik_node* node = (struct ik_node*)
        ik_refcounted_alloc(sizeof *node, (ik_deinit_func)ik_node_deinit);
    if (node == NULL)
        return NULL;

    node->user = user;
    node->algorithm = NULL;
    node->constraint = NULL;
    node->effector = NULL;
    node->pole = NULL;

    ik_vec3_set_zero(node->position.f);
    ik_quat_set_identity(node->rotation.f);

    node->rotation_weight = 1.0;
    node->mass = 1.0;

    node->parent = NULL;
    btree_init(&node->children, sizeof(struct ik_node*));

    return node;
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

    add_child_failed    : ik_node_destroy(child);
    create_child_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
int
ik_node_link(struct ik_node* node, struct ik_node* child)
{
    assert(node);
    assert(child);
    assert(child != node);

    /* Searches the entire tree for the child guid -- disabled in release mode
     * for performance reasons */
#ifdef DEBUG
    {
        struct ik_node* root = node;
        while (root->parent)
            root = root->parent;
        if (ik_node_find(root, ik_guid(GUID(child))) != NULL)
            ik_log_printf(IK_WARN, "Child guid %d already exists in the tree! It will be inserted, but find_child() will only find one of the two.", GUID(child));
    }
#endif

    if (btree_insert_new(&node->children, GUID(child), &child) != BTREE_OK)
    {
        ik_log_printf(IK_ERROR, "Child guid %d already exists in this node's list of children! Node was not inserted into the tree.", GUID(child));
        return -1;
    }

    /* May already be part of a tree */
    IK_INCREF(child);
    ik_node_unlink(child);
    child->parent = node;

    return 0;
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
    IK_DECREF(node);
}

/* ------------------------------------------------------------------------- */
uint32_t
ik_node_count(const struct ik_node* node)
{
    uint32_t count = 1;
    NODE_FOR_EACH(node, user, child)
        count += ik_node_count(child);
    NODE_END_EACH
    return count;
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_node_pack(const struct ik_node* root)
{
    /*uint32_t node_count = ik_node_count(root);
    struct ik_node* new_root = (struct ik_node*)
        ik_refcounted_alloc_array(sizeof *new_root, (ik_deinit_func)ik_node_deinit, node_count);*/

    return (struct ik_node*)root;
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
#define X1(upper, lower, argtype)                                             \
    struct ik_##lower*                                                        \
    ik_node_create_##lower(struct ik_node* node, argtype arg) {               \
        struct ik_##lower* lower = ik_##lower##_create(arg);                  \
        if (lower == NULL)                                                    \
            return NULL;                                                      \
                                                                              \
        ik_node_attach_##lower(node, lower);                                  \
        return lower;                                                         \
    }
#define X(upper, lower)                                                       \
    struct ik_##lower*                                                        \
    ik_node_create_##lower(struct ik_node* node) {                            \
        struct ik_##lower* lower = ik_##lower##_create();                     \
        if (lower == NULL)                                                    \
            return NULL;                                                      \
                                                                              \
        ik_node_attach_##lower(node, lower);                                  \
        return lower;                                                         \
    }
IK_ATTACHMENT_LIST
#undef X
#undef X1

/* ------------------------------------------------------------------------- */
#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower)                                                       \
    void                                                                      \
    ik_node_attach_##lower(struct ik_node* node, struct ik_##lower* lower) {  \
        IK_INCREF(lower);                                                     \
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
        if (lower) {                                                          \
            lower->node = NULL;                                               \
            IK_DECREF(lower);                                                 \
        }                                                                     \
        return lower;                                                         \
    }

IK_ATTACHMENT_LIST
#undef X
#undef X1

struct ik_node_state
{
    union ik_quat rotation;
    union ik_vec3 position;
};

#define IK_POSE_OFFSET \
    IK_ALIGN_TO_CPU_WORD_SIZE(sizeof(struct ik_pose))

/* ------------------------------------------------------------------------- */
struct ik_pose*
ik_pose_alloc(const struct ik_node* root)
{
    uint32_t node_count = ik_node_count(root);
    struct ik_pose* state = (struct ik_pose*)ik_refcounted_alloc(
        IK_POSE_OFFSET + sizeof(struct ik_node_state) * node_count, NULL);
    if (state == NULL)
        return NULL;

#ifdef DEBUG
    state->node_count = node_count;
#endif

    return state;
}

/* ------------------------------------------------------------------------- */
static void
save_pose(const struct ik_node* node, struct ik_node_state** data)
{
    NODE_FOR_EACH(node, user, child)
        save_pose(child, data);
    NODE_END_EACH

    (*data)->position = node->position;
    (*data)->rotation = node->rotation;
    (*data)++;
}
void
ik_pose_save(struct ik_pose* state, const struct ik_node* root)
{
    struct ik_node_state* data = (struct ik_node_state*)((uintptr_t)state + IK_POSE_OFFSET);
#ifdef DEBUG
    assert(state->node_count == ik_node_count(root));
#endif
    save_pose(root, &data);
}

/* ------------------------------------------------------------------------- */
static void
restore_pose(struct ik_node* node, struct ik_node_state** data)
{
    NODE_FOR_EACH(node, user, child)
        restore_pose(child, data);
    NODE_END_EACH

    node->position = (*data)->position;
    node->rotation = (*data)->rotation;
    (*data)++;
}
void
ik_pose_apply(const struct ik_pose* state, struct ik_node* root)
{
    struct ik_node_state* data = (struct ik_node_state*)((uintptr_t)state + IK_POSE_OFFSET);
#ifdef DEBUG
    assert(state->node_count == ik_node_count(root));
#endif
    restore_pose(root, &data);
}
