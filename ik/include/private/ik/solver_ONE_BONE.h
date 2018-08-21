#ifndef IK_SOLVER_ONE_BONE_H
#define IK_SOLVER_ONE_BONE_H

#include "ik/config.h"
#include "ik/solver.h"

C_BEGIN

uintptr_t
ik_solver_ONE_BONE_type_size(void);

ikret_t
ik_solver_ONE_BONE_construct(struct ik_solver_t* solver);

void
ik_solver_ONE_BONE_destruct(struct ik_solver_t* solver);

ikret_t
ik_solver_ONE_BONE_rebuild(struct ik_solver_t* solver);

ikret_t
ik_solver_ONE_BONE_solve(struct ik_solver_t* solver);

C_END

#endif /* IK_SOLVER_ONE_BONE_H */
