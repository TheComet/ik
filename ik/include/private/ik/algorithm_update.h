#ifndef IK_ALGORITHM_UPDATE
#define IK_ALGORITHM_UPDATE

#include "ik/config.h"

C_BEGIN

struct vector_t;

IK_PRIVATE_API void
ik_algorithm_update_effector_targets(struct ik_algorithm_t* algorithm);

IK_PRIVATE_API void
ik_algorithm_update_node_distances(struct ik_algorithm_t* algorithm);

C_END

#endif /* IK_ALGORITHM_UPDATE */
