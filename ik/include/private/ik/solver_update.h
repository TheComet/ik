#ifndef IK_SOLVER_UPDATE
#define IK_SOLVER_UPDATE

#include "ik/config.h"

C_BEGIN

struct vector_t;

IK_PRIVATE_API void
ik_solver_update_effector_targets(struct vector_t* ntf_list);

IK_PRIVATE_API void
ik_solver_update_node_distances(struct vector_t* ntf_list);

C_END

#endif /* IK_SOLVER_UPDATE */
