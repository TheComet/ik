
#ifndef IK_SUBTREE_H
#define IK_SUBTREE_H

#include "ik/config.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_subtree
{
    const struct ik_node* root;
    struct cs_vector leaves;  /* list of ik_node* */
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
subtree_check_node(const struct ik_subtree* st, const struct ik_node* node);

int
subtree_check_children_up_to(const struct ik_subtree* st,
                             const struct ik_node* node,
                             int max_count);

#define subtree_leaves(st) \
    (vector_count(&(st)->leaves))

#define subtree_get_leaf(st, idx) \
    (*(struct ik_node**)vector_get_element(&(st)->leaves, idx))

C_END

#endif /* IK_SUBTREE_H */
