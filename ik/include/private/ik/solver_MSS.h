#ifndef IK_SOLVER_MSS_H
#define IK_SOLVER_MSS_H

#include "ik/config.h"
#include "ik/solver.h"

C_BEGIN

uintptr_t
ik_solver_MSS_type_size(void);

ikret_t
ik_solver_MSS_construct(struct ik_solver_t* solver);

void
ik_solver_MSS_destruct(struct ik_solver_t* solver);

ikret_t
ik_solver_MSS_rebuild(struct ik_solver_t* solver);

ikret_t
ik_solver_MSS_solve(struct ik_solver_t* solver);

C_END

#endif /* IK_SOLVER_MSS_H */
