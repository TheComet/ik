#include "ik/config.h"

C_HEADER_BEGIN

int
solver_MSD_construct(ik_solver_t* solver);

void
solver_MSD_destruct(ik_solver_t* solver);

int
solver_MSD_solve(ik_solver_t* solver);

C_HEADER_END
