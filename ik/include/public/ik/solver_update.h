#ifndef IK_ALGORITHM_UPDATE
#define IK_ALGORITHM_UPDATE

#include "ik/config.h"

C_BEGIN

struct vector_t;

IK_PRIVATE_API void
ik_solver_update_effector_targets(struct ik_solver_t* solver);

IK_PRIVATE_API void
ik_solver_update_node_distances(struct ik_solver_t* solver);

C_END

#endif /* IK_ALGORITHM_UPDATE */
