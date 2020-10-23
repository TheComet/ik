#pragma once

#include "ik/refcount.h"

C_BEGIN

struct ik_bone;

struct ik_pose
{
    IK_REFCOUNTED_HEAD
#if !defined(NDEBUG)
    uint32_t bone_count;
#endif
};

IK_PUBLIC_API struct ik_pose*
ik_pose_alloc(const struct ik_bone* root);

IK_PUBLIC_API void
ik_pose_save(struct ik_pose* state, const struct ik_bone* root);

IK_PUBLIC_API void
ik_pose_apply(const struct ik_pose* state, struct ik_bone* root);

C_END
