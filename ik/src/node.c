#include "ik/node.h"
#include "ik/quat.inl"
#include "ik/vec3.inl"
#include <stddef.h>

/* ------------------------------------------------------------------------- */
static void
ik_node_destroy(struct ik_node* node)
{
    node->refcount->deinit((struct ik_refcounted*)node);
    ik_refcounted_obj_free((struct ik_refcounted*)node);
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_node_create(void)
{
    struct ik_node* node = (struct ik_node*)
        ik_tree_object_create(sizeof *node);
    if (node == NULL)
        return NULL;

    ik_node_init(node);

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
void
ik_node_init(struct ik_node* node)
{
    ik_vec3_set_zero(node->position.f);
    ik_quat_set_identity(node->rotation.f);
}
