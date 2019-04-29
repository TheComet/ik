#ifndef IK_JOBLIST_H
#define IK_JOBLIST_H

#include "cstructures/vector.h"
#include "ik/config.h"

C_BEGIN

struct ik_node_t;

struct ik_joblist_t
{
    struct vector_t ndv_list  /* ik_node_data_view_t */;
    uint32_t highest_child_count;
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
