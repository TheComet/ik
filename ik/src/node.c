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

/* ------------------------------------------------------------------------- */
static void
ik_node_deinit(struct ik_node* node)
{
    assert(node);

    /* Unref children */
    NODE_FOR_EACH(node, child)
        IK_DECREF(child);
    NODE_END_EACH

    /* unref attachments */
#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower) ik_node_detach_##lower(node);
    IK_ATTACHMENT_LIST
#undef X
#undef X1

    /* deinit */
    vector_deinit(&node->children);
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
ik_node_create(void)
{
    struct ik_node* node = (struct ik_node*)
        ik_refcounted_alloc(sizeof *node, (ik_deinit_func)ik_node_deinit);
    if (node == NULL)
        return NULL;

    node->user_data = NULL;
    node->algorithm = NULL;
    node->constraint = NULL;
    node->effector = NULL;
    node->pole = NULL;

    ik_vec3_set_zero(node->position.f);
    ik_quat_set_identity(node->rotation.f);

    node->rotation_weight = 1.0;
    node->mass = 1.0;

    node->parent = NULL;
    vector_init(&node->children, sizeof(struct ik_node*));

    return node;
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_node_create_child(struct ik_node* parent)
{
    struct ik_node* child;
    if ((child = ik_node_create()) == NULL)
        goto create_child_failed;
    if (ik_node_link(parent, child) != 0)
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

    if (vector_push(&node->children, &child) != 0)
    {
        ik_log_out_of_memory("ik_node_link()");
        return -1;
    }

    /* May already be part of a tree */
    IK_INCREF(child);
    ik_node_unlink(child);
    child->parent = node;

    return 0;
}

/* ------------------------------------------------------------------------- */
int
ik_node_can_link(const struct ik_node* parent, const struct ik_node* child)
{
    do {
        if (parent == child)
            return 0;
        parent = parent->parent;
    } while(parent);

    return 1;
}

/* ------------------------------------------------------------------------- */
void
ik_node_unlink(struct ik_node* node)
{
    cs_vec_idx idx;
    assert(node);

    if (node->parent == NULL)
        return;

    idx = vector_find_element(&node->parent->children, &node);
    if (idx != vector_count(&node->parent->children))
        vector_erase_index(&node->parent->children, idx);

    node->parent = NULL;
    IK_DECREF(node);
}

/* ------------------------------------------------------------------------- */
void
ik_node_unlink_all_children(struct ik_node* node)
{
    NODE_FOR_EACH(node, child)
        child->parent = NULL;
        IK_DECREF(child);
    NODE_END_EACH
    vector_clear(&node->children);
}

/* ------------------------------------------------------------------------- */
uint32_t
ik_node_count(const struct ik_node* node)
{
    uint32_t count = 1;
    NODE_FOR_EACH(node, child)
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
ik_node_find(struct ik_node* node, const void* user_data)
{
    /* might be searching for this node? */
    if (node->user_data == user_data)
        return (struct ik_node*)node;

    NODE_FOR_EACH(node, child)
        struct ik_node* found_child;
        if ((found_child = ik_node_find(child, user_data)) != NULL)
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
