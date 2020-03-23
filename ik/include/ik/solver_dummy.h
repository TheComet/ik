#ifndef IK_SOLVER_DUMMY
#define IK_SOLVER_DUMMY

#include "ik/config.h"
#include "ik/solver.h"

C_BEGIN

struct ik_solver_dummy1_t
{
    IK_SOLVER_HEAD
};

IK_PRIVATE_API IKRET
ik_solver_dummy1_init(struct ik_solver_dummy1_t* solver);

IK_PRIVATE_API void
ik_solver_dummy1_deinit(struct ik_solver_dummy1_t* solver);

IK_PRIVATE_API IKRET
ik_solver_dummy1_prepare(struct ik_solver_dummy1_t* solver);

IK_PRIVATE_API IKRET
ik_solver_dummy1_solve(struct ik_solver_dummy1_t* solver);

struct ik_solver_dummy2_t
{
    IK_SOLVER_HEAD
};

IK_PRIVATE_API IKRET
ik_solver_dummy2_init(struct ik_solver_dummy2_t* solver);

IK_PRIVATE_API void
ik_solver_dummy2_deinit(struct ik_solver_dummy2_t* solver);

IK_PRIVATE_API IKRET
ik_solver_dummy2_prepare(struct ik_solver_dummy2_t* solver);

IK_PRIVATE_API IKRET
ik_solver_dummy2_solve(struct ik_solver_dummy2_t* solver);

C_END

#endif /* IK_SOLVER_DUMMY */
