#ifndef IK_SOLVER_ONE_BONE_H
#define IK_SOLVER_ONE_BONE_H

#include "ik/config.h"
#include "ik/solver.h"

C_BEGIN

#if defined(IK_BUILDING)

IK_PRIVATE_API uintptr_t
ik_solver_ONE_BONE_type_size(void);

IK_PRIVATE_API ikret_t
ik_solver_ONE_BONE_construct(struct ik_solver_t* solver);

IK_PRIVATE_API void
ik_solver_ONE_BONE_destruct(struct ik_solver_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_ONE_BONE_prepare(struct ik_solver_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_ONE_BONE_solve(struct ik_solver_t* solver);

#endif /* IK_BUILDING */

C_END

#endif /* IK_SOLVER_ONE_BONE_H */
