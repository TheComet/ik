#pragma once

#include "ik/config.h"

C_BEGIN

struct ik_node;
struct ik_bone;

IK_PUBLIC_API void
ik_node_to_bone(const struct ik_node* node_root, struct ik_bone* bone_root);

IK_PUBLIC_API void
ik_bone_to_node(const struct ik_bone* bone_root, struct ik_node* node_root);

IK_PUBLIC_API struct ik_bone*
ik_node_to_bone_inplace(struct ik_node* root);

IK_PUBLIC_API struct ik_node*
ik_bone_to_node_inplace(struct ik_bone* root);

C_END
