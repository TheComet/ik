#include "ik/effector.h"
#include "ik/tree_object.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"
#include <stddef.h>

/* ------------------------------------------------------------------------- */
struct ik_effector*
ik_effector_create(void)
{
    struct ik_effector* eff = (struct ik_effector*)
        ik_attachment_alloc(sizeof *eff, NULL);
    if (eff == NULL)
        return NULL;

    ik_vec3_set_zero(eff->target_position.f);
    ik_quat_set_identity(eff->target_rotation.f);
    eff->weight = 1.0;
    eff->rotation_weight = 1.0;
    eff->rotation_decay = 0.25;
    eff->chain_length = 0;
    eff->features = 0;

    return eff;
}

/* ------------------------------------------------------------------------- */
struct ik_effector*
ik_effector_duplicate(const struct ik_effector* effector)
{
    struct ik_effector* dup = (struct ik_effector*)
        ik_attachment_alloc(sizeof *dup, NULL);
    if (dup == NULL)
        return NULL;

    dup->target_position = effector->target_position;
    dup->target_rotation = effector->target_rotation;
    dup->weight = effector->weight;
    dup->rotation_weight = effector->rotation_weight;
    dup->rotation_decay = effector->rotation_decay;
    dup->chain_length = effector->chain_length;
    dup->features = effector->features;

    return dup;
}

/* ------------------------------------------------------------------------- */
static int
count_effectors(const struct ik_tree_object* root)
{
    int count = root->effector ? 1 : 0;
    TREE_OBJECT_FOR_EACH_CHILD(root, child)
        count += count_effectors(child);
    TREE_OBJECT_END_EACH
    return count;
}

/* ------------------------------------------------------------------------- */
static void
copy_from_tree(struct ik_effector** eff_buf,
               struct ik_tree_object* dst,
               const struct ik_tree_object* src)
{
    uint32_t i;

    if (src->effector)
    {
        struct ik_effector* eff = *eff_buf;
        (*eff_buf)++;

        ik_attachment_init((struct ik_attachment*)eff);

        eff->target_position = src->effector->target_position;
        eff->target_rotation = src->effector->target_rotation;
        eff->weight = src->effector->weight;
        eff->rotation_weight = src->effector->rotation_weight;
        eff->rotation_decay = src->effector->rotation_decay;
        eff->chain_length = src->effector->chain_length;
        eff->features = src->effector->features;

        ik_tree_object_attach_effector(dst, eff);
    }

    assert(ik_tree_object_child_count(src) == ik_tree_object_child_count(dst));
    for (i = 0; i != ik_tree_object_child_count(src); ++i)
    {
        copy_from_tree(eff_buf,
                       ik_tree_object_get_child(dst, i),
                       ik_tree_object_get_child(src, i));
    }
}

/* ------------------------------------------------------------------------- */
int
ik_effector_duplicate_from_tree(struct ik_tree_object* dst,
                                const struct ik_tree_object* src)
{
    int count;
    struct ik_effector* eff_buf;

    count = count_effectors(src);
    if (count == 0)
        return 0;

    eff_buf = (struct ik_effector*)
        ik_refcounted_alloc_array(sizeof *eff_buf, NULL, count);
    if (eff_buf == NULL)
        return -1;

    copy_from_tree(&eff_buf, dst, src);
    return 0;
}
