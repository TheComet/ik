#include "ik/solver_base.h"

IK_IMPLEMENT(node_MSS, node_base)
{
}

IK_IMPLEMENT(solver_MSS, solver_base)
{
    IK_OVERRIDE(type_size)
    IK_CONSTRUCTOR(construct)
    IK_DESTRUCTOR(destruct)
    IK_AFTER(rebuild_data)
    IK_AFTER(solve)
}

/*
 * Because we use X macros to fill in the ik interface struct, we have to
 * generate the implementation defines for the effector and constraint
 * interfaces as well. These don't actually override anything.
 */
IK_IMPLEMENT(effector_MSS, effector_base)
IK_IMPLEMENT(constraint_MSS, constraint_base)

/*
 * Need to combine multiple ik_ret return values from the various before/after
 * functions.
 */
static inline ik_ret ik_solver_MSS_harness_rebuild_data_return_value(ik_ret a, ik_ret b) {
    if (a != IK_OK) return a;
    return b;
}
static inline ik_ret ik_solver_MSS_harness_solve_return_value(ik_ret a, ik_ret b) {
    if (a != IK_OK) return a;
    return b;
}
