#ifndef IK_SOLVER_FABRIK_H
#define IK_SOLVER_FABRIK_H

#include "ik/config.h"
#include "ik/solver.h"

C_BEGIN

uintptr_t
ik_solver_FABRIK_type_size(void);

ikret_t
ik_solver_FABRIK_construct(struct ik_solver_t* solver);

void
ik_solver_FABRIK_destruct(struct ik_solver_t* solver);

ikret_t
ik_solver_FABRIK_rebuild(struct ik_solver_t* solver);

ikret_t
ik_solver_FABRIK_solve(struct ik_solver_t* solver);

C_END

#endif /* IK_SOLVER_FABRIK_H */
