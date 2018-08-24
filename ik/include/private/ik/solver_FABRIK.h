#ifndef IK_SOLVER_FABRIK_H
#define IK_SOLVER_FABRIK_H

#include "ik/config.h"
#include "ik/solver.h"

C_BEGIN

#if defined(IK_BUILDING)

IK_PRIVATE_API uintptr_t
ik_solver_FABRIK_type_size(void);

IK_PRIVATE_API ikret_t
ik_solver_FABRIK_construct(struct ik_solver_t* solver);

IK_PRIVATE_API void
ik_solver_FABRIK_destruct(struct ik_solver_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_FABRIK_rebuild(struct ik_solver_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_FABRIK_solve(struct ik_solver_t* solver);

#endif /* IK_BUILDING */

C_END

#endif /* IK_SOLVER_FABRIK_H */
