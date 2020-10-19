#pragma once

#include "ik/refcount.h"

C_BEGIN

struct ik_node;

struct ik_pose
{
    IK_REFCOUNTED_HEAD
#if !defined(NDEBUG)
    uint32_t node_count;
#endif
};

IK_PUBLIC_API struct ik_pose*
ik_pose_alloc(const struct ik_node* root);

IK_PUBLIC_API void
ik_pose_save(struct ik_pose* state, const struct ik_node* root);

IK_PUBLIC_API void
ik_pose_apply(const struct ik_pose* state, struct ik_node* root);

C_END
