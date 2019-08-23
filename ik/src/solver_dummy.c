#include "ik/solver_dummy.h"
#include "ik/solver.h"

/* ------------------------------------------------------------------------- */
ikret_t   ik_solver_dummy1_init(struct ik_solver_dummy1_t* solver)    { return IK_OK; }
void      ik_solver_dummy1_deinit(struct ik_solver_dummy1_t* solver)  {}
ikret_t   ik_solver_dummy1_prepare(struct ik_solver_dummy1_t* solver) { return IK_OK; }
ikret_t   ik_solver_dummy1_solve(struct ik_solver_dummy1_t* solver)   { return IK_OK; }

/* ------------------------------------------------------------------------- */
ikret_t   ik_solver_dummy2_init(struct ik_solver_dummy2_t* solver)    { return IK_OK; }
void      ik_solver_dummy2_deinit(struct ik_solver_dummy2_t* solver)  {}
ikret_t   ik_solver_dummy2_prepare(struct ik_solver_dummy2_t* solver) { return IK_OK; }
ikret_t   ik_solver_dummy2_solve(struct ik_solver_dummy2_t* solver)   { return IK_OK; }
