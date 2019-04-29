#ifndef IK_ALGORITHM_MSS_H
#define IK_ALGORITHM_MSS_H

#include "ik/config.h"
#include "ik/solver.h"

C_BEGIN

#if defined(IK_BUILDING)

IK_PRIVATE_API uintptr_t
ik_solver_MSS_type_size(void);

IK_PRIVATE_API ikret_t
ik_solver_MSS_init(struct ik_solver_t* solver);

IK_PRIVATE_API void
ik_solver_MSS_deinit(struct ik_solver_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_MSS_rebuild(struct ik_solver_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_MSS_solve(struct ik_solver_t* solver);

#endif /* IK_BUILDING */

C_END

#endif /* IK_ALGORITHM_MSS_H */
