#ifndef IK_SOLVER_2BONE_H
#define IK_SOLVER_2BONE_H

#include "ik/config.h"

C_HEADER_BEGIN

int
solver_2bone_construct(ik_solver_t* solver);

void
solver_2bone_destruct(ik_solver_t* solver);

int
solver_2bone_post_chain_build(ik_solver_t* solver);

int
solver_2bone_solve(ik_solver_t* solver);

C_HEADER_END

#endif /* IK_SOLVER_2BONE_H */
