#ifndef IK_JOBLIST_H
#define IK_JOBLIST_H

#include "ik/config.h"
#include "ik/node_data.h"

C_BEGIN

struct ik_joblist_t
{
    struct vector_t solver_list;  /* ik_solver_t* */
};

/*!
 * @brief Flattens all relevant node data in a tree.
 */
IK_PUBLIC_API IKRET
ik_joblist_create(struct ik_joblist_t** joblist);

IK_PUBLIC_API IKRET
ik_joblist_init(struct ik_joblist_t* joblist);

IK_PUBLIC_API void
ik_joblist_deinit(struct ik_joblist_t* joblist);

IK_PUBLIC_API void
ik_joblist_free(struct ik_joblist_t* joblist);

IK_PUBLIC_API IKRET
ik_joblist_update(struct ik_joblist_t* joblist, struct ik_node_t* root);

C_END

#endif /* IK_JOBLIST_H */
