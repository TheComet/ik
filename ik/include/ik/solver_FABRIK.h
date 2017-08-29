#ifndef IK_SOLVER_FABRIK_H
#define IK_SOLVER_FABRIK_H

#include "ik/config.h"

C_HEADER_BEGIN

int
solver_FABRIK_construct(ik_solver_t* solver);

void
solver_FABRIK_destruct(ik_solver_t* solver);

int
solver_FABRIK_solve(ik_solver_t* solver);

C_HEADER_END

#endif /* IK_SOLVER_FABRIK_H */
