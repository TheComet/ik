#ifndef IK_SOLVER_B2_H
#define IK_SOLVER_B2_H

#include "ik/config.h"
#include "ik/solver.h"

C_BEGIN

struct ik_solver_b2_t
{
    IK_SOLVER_HEAD
};

IK_PRIVATE_API ikret_t
ik_solver_b2_init(struct ik_solver_b2_t* solver);

IK_PRIVATE_API void
ik_solver_b2_deinit(struct ik_solver_b2_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_b2_prepare(struct ik_solver_b2_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_b2_solve(struct ik_solver_b2_t* solver);

C_END

#endif /* IK_SOLVER_B2_H */
