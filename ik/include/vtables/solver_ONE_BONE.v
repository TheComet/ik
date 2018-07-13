#include "ik/solver_base.h"

IK_IMPLEMENT(solver_ONE_BONE, solver_base)
{
    IK_OVERRIDE(type_size)
    IK_CONSTRUCTOR(construct)
    IK_DESTRUCTOR(destruct)
    IK_AFTER(rebuild)
    IK_AFTER(solve)
}

/*
 * Because we use X macros to fill in the ik interface struct, we have to
 * generate the implementation defines for the node, effector and constraint
 * interfaces as well. These don't actually override anything.
 */
IK_IMPLEMENT(node_ONE_BONE, node_base)
IK_IMPLEMENT(effector_ONE_BONE, effector_base)
IK_IMPLEMENT(constraint_ONE_BONE, constraint_base)
IK_IMPLEMENT(pole_ONE_BONE, pole_base)

/*
 * Need to combine multiple ikret_t return values from the various before/after
 * functions.
 */
static inline ikret_t ik_solver_ONE_BONE_harness_rebuild_return_value(ikret_t a, ikret_t b) {
    if (a != IK_OK) return a;
    return b;
}
static inline ikret_t ik_solver_ONE_BONE_harness_solve_return_value(ikret_t a, ikret_t b) {
    if (a != IK_OK) return a;
    return b;
}
