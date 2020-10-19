#include "ik/bone.h"
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
ik_bone_create(void)
{
    struct ik_bone* bone = (struct ik_bone*)
        ik_tree_object_create(sizeof *bone);
    if (bone == NULL)
        return NULL;

    ik_vec3_set_zero(bone->position.f);
    ik_quat_set_identity(bone->rotation.f);
    bone->length = 0.0;

    return bone;
}

/* ------------------------------------------------------------------------- */
struct ik_bone*
ik_bone_create_child(struct ik_bone* parent)
{
    struct ik_bone* child;
    if ((child = ik_bone_create()) == NULL)
        goto create_child_failed;
    if (ik_bone_link(parent, child) != 0)
        goto add_child_failed;

    return child;

    add_child_failed    : ik_bone_destroy(child);
    create_child_failed : return NULL;
}
