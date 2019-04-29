#ifndef IK_ALGORITHM_ONE_BONE_H
#define IK_ALGORITHM_ONE_BONE_H

#include "ik/config.h"
#include "ik/solver.h"

C_BEGIN

struct ik_solver_b1_t
{
    IK_ALGORITHM_HEAD
};

IK_PRIVATE_API uintptr_t
ik_solver_b1_type_size(void);

IK_PRIVATE_API ikret_t
ik_solver_b1_init(struct ik_solver_b1_t* solver);

IK_PRIVATE_API void
ik_solver_b1_deinit(struct ik_solver_b1_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_b1_prepare(struct ik_solver_b1_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_b1_solve(struct ik_solver_b1_t* solver);

C_END

#endif /* IK_ALGORITHM_ONE_BONE_H */
