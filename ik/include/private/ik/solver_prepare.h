#ifndef IK_SOLVER_PREPARE_H
#define IK_SOLVER_PREPARE_H

#include "ik/config.h"

C_BEGIN

struct ik_solver_t;

IK_PRIVATE_API ikret_t
ik_solver_prepare_stack_buffer(struct ik_solver_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_prepare_effector_chains(struct ik_solver_t* solver);

IK_PRIVATE_API void
ik_solver_prepare_pole_targets(struct ik_solver_t* solver);

C_END

#endif /* IK_SOLVER_PREPARE_H */
