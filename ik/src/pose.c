#include "ik/pose.h"
#include "ik/bone.h"
#include <stddef.h>

struct ik_bone_state
{
    union ik_quat rotation;
    union ik_vec3 position;
};

#define IK_POSE_OFFSET \
    IK_ALIGN_TO_CPU_WORD_SIZE(sizeof(struct ik_pose))

/* ------------------------------------------------------------------------- */
struct ik_pose*
ik_pose_alloc(const struct ik_bone* root)
{
    uint32_t bone_count = ik_bone_count(root);
    struct ik_pose* state = (struct ik_pose*)ik_refcounted_alloc(
        IK_POSE_OFFSET + sizeof(struct ik_bone_state) * bone_count, NULL);
    if (state == NULL)
        return NULL;

#if !defined(NDEBUG)
    state->bone_count = bone_count;
#endif

    return state;
}

/* ------------------------------------------------------------------------- */
static void
save_pose(const struct ik_bone* bone, struct ik_bone_state** data)
{
    BONE_FOR_EACH_CHILD(bone, child)
        save_pose(child, data);
    BONE_END_EACH

    (*data)->position = bone->position;
    (*data)->rotation = bone->rotation;
    (*data)++;
}
void
ik_pose_save(struct ik_pose* state, const struct ik_bone* root)
{
    struct ik_bone_state* data = (struct ik_bone_state*)((uintptr_t)state + IK_POSE_OFFSET);
#if !defined(NDEBUG)
    assert(state->bone_count == ik_bone_count(root));
#endif
    save_pose(root, &data);
}

/* ------------------------------------------------------------------------- */
static void
restore_pose(struct ik_bone* bone, struct ik_bone_state** data)
{
    BONE_FOR_EACH_CHILD(bone, child)
        restore_pose(child, data);
    BONE_END_EACH

    bone->position = (*data)->position;
    bone->rotation = (*data)->rotation;
    (*data)++;
}
void
ik_pose_apply(const struct ik_pose* state, struct ik_bone* root)
{
    struct ik_bone_state* data = (struct ik_bone_state*)((uintptr_t)state + IK_POSE_OFFSET);
#if !defined(NDEBUG)
    assert(state->bone_count == ik_bone_count(root));
#endif
    restore_pose(root, &data);
}
