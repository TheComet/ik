#ifndef IK_SUBTREE_H
#define IK_SUBTREE_H

#include "ik/config.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_subtree
{
    const struct ik_node* root;
    struct vector_t leaves;  /* list of ik_node* */
};

IK_PRIVATE_API int
subtree_init(struct ik_subtree* st);

IK_PRIVATE_API void
subtree_deinit(struct ik_subtree* st);

IK_PRIVATE_API void
subtree_set_root(struct ik_subtree* st, const struct ik_node* root);

IK_PRIVATE_API int
subtree_add_leaf(struct ik_subtree* st, const struct ik_node* leaf);

IK_PRIVATE_API int
subtree_is_leaf_node(const struct ik_subtree* st, const struct ik_node* node);

IK_PRIVATE_API int
subtree_is_part_of(const struct ik_subtree* st, const struct ik_node* node);

IK_PRIVATE_API int
subtree_two_or_more_children_are_part_of(const struct ik_subtree* st, const struct ik_node* node);

C_END

#endif /* IK_SUBTREE_H */
