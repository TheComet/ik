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
ik_subtree_init(struct ik_subtree* st);

IK_PRIVATE_API void
ik_subtree_deinit(struct ik_subtree* st);

C_END

#endif /* IK_SUBTREE_H */
