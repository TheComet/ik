#include "ik/solver_base.h"

IK_IMPLEMENT(solver_FABRIK, solver_base)
{
    IK_OVERRIDE(type_size)
    IK_CONSTRUCTOR(construct)
    IK_BEFORE(destruct)
    IK_AFTER(solve)
}

/*
 * Because we use X macros to fill in the ik interface struct, we have to
 * generate the implementation defines for the effector and constraint
 * interfaces as well. These don't actually override anything.
 */
IK_IMPLEMENT(effector_FABRIK, effector_base)
IK_IMPLEMENT(constraint_FABRIK, constraint_base)

/*
 * Need to combine multiple ikret_t return values from the various before/after
 * functions.
 */
static inline ikret_t ik_solver_FABRIK_harness_solve_return_value(ikret_t a, ikret_t b) {
    if (a != IK_OK) return a;
    return b;
}
