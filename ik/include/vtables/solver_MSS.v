#include "ik/solver_base.h"

IK_IMPLEMENT(node_MSS, node_base)
{
}

IK_IMPLEMENT(solver_MSS, solver_base)
{
    IK_OVERRIDE(type_size)
    IK_CONSTRUCTOR(construct)
    IK_DESTRUCTOR(destruct)
    IK_AFTER(rebuild)
    IK_AFTER(solve)
}

/*
 * Because we use X macros to fill in the ik interface struct, we have to
 * generate the implementation defines for the effector and constraint
 * interfaces as well. These don't actually override anything.
 */
IK_IMPLEMENT(effector_MSS, effector_base)
IK_IMPLEMENT(constraint_MSS, constraint_base)
IK_IMPLEMENT(pole_MSS, pole_base)

/*
 * Need to combine multiple ikret_t return values from the various before/after
 * functions.
 */
static inline ikret_t ik_solver_MSS_harness_rebuild_return_value(ikret_t a, ikret_t b) {
    if (a != IK_OK) return a;
    return b;
}
static inline ikret_t ik_solver_MSS_harness_solve_return_value(ikret_t a, ikret_t b) {
    if (a != IK_OK) return a;
    return b;
}
