#pragma once

#include "ik/config.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_bone;

struct ik_subtree
{
    struct ik_bone* root;
    struct cs_vector leaves;  /* list of ik_bone* */
};

IK_PRIVATE_API void
subtree_init(struct ik_subtree* st);

IK_PRIVATE_API void
subtree_deinit(struct ik_subtree* st);

IK_PRIVATE_API void
subtree_set_root(struct ik_subtree* st, struct ik_bone* root);

IK_PRIVATE_API int
subtree_add_leaf(struct ik_subtree* st, struct ik_bone* leaf);

IK_PRIVATE_API int
subtree_is_leaf_bone(const struct ik_subtree* st, const struct ik_bone* bone);

IK_PRIVATE_API int
subtree_check_bone(const struct ik_subtree* st, const struct ik_bone* bone);

int
subtree_check_children_up_to(const struct ik_subtree* st,
                             const struct ik_bone* bone,
                             int max_count);

#define subtree_leaves(st) \
    (vector_count(&(st)->leaves))

#define subtree_get_leaf(st, idx) \
    (*(struct ik_bone**)vector_get_element(&(st)->leaves, idx))

#define subtree_get_root(st) \
    ((st)->root)

C_END
