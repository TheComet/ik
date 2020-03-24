#ifndef IK_JOBLIST_H
#define IK_JOBLIST_H

#include "ik/config.h"
#include "ik/refcount.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_node;

struct ik_joblist
{
    IK_REFCOUNT_HEAD
    struct vector_t solvers;  /* list of ik_solver* */
};

/*!
 * @brief Flattens all relevant node data in a tree.
 */
IK_PUBLIC_API struct ik_joblist*
ik_joblist_create(const struct ik_node* root);

IK_PUBLIC_API ikret
ik_joblist_update(struct ik_joblist* joblist, const struct ik_node* root);

IK_PUBLIC_API ikret
ik_joblist_merge(struct ik_joblist* dst, const struct ik_joblist* src);

IK_PUBLIC_API ikret
ik_joblist_execute(const struct ik_joblist* joblist);

C_END

#endif /* IK_JOBLIST_H */
