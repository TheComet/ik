#include "ik/bone.h"
#include "ik/node.h"
#include "ik/quat.inl"
#include "ik/vec3.inl"
#include <stddef.h>

/* ------------------------------------------------------------------------- */
static void
ik_bone_destroy(struct ik_bone* bone)
{
    bone->refcount->deinit((struct ik_refcounted*)bone);
    ik_refcounted_obj_free((struct ik_refcounted*)bone);
}

/* ------------------------------------------------------------------------- */
struct ik_bone*
ik_bone_create(ikreal length)
{
    struct ik_bone* bone = (struct ik_bone*)
        ik_tree_object_create(sizeof *bone);
    if (bone == NULL)
        return NULL;

    ik_vec3_set_zero(bone->position.f);
    ik_quat_set_identity(bone->rotation.f);
    bone->length = length;

    return bone;
}

/* ------------------------------------------------------------------------- */
struct ik_bone*
ik_bone_create_child(struct ik_bone* parent, ikreal length)
{
    struct ik_bone* child;
    if ((child = ik_bone_create(length)) == NULL)
        goto create_child_failed;
    if (ik_bone_link(parent, child) != 0)
        goto add_child_failed;

    return child;

    add_child_failed    : ik_bone_destroy(child);
    create_child_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_bone_duplicate_shallow_for_node_transform(const struct ik_bone* root)
{
    return (struct ik_node*)ik_tree_object_duplicate_shallow(
        (const struct ik_tree_object*)root, sizeof(struct ik_node), 1);
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_bone_duplicate_full_for_node_transform(const struct ik_bone* root)
{
    return (struct ik_node*)ik_tree_object_duplicate_full(
        (const struct ik_tree_object*)root, sizeof(struct ik_node), 1);
}

/* ------------------------------------------------------------------------- */
void
ik_bone_head_position(const struct ik_bone* bone, ikreal v[3])
{
    ik_vec3_direction_of(v, bone->rotation.f);
    ik_vec3_mul_scalar(v, bone->length);
}
