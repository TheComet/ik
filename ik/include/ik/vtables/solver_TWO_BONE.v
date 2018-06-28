#include "ik/solver_base.h"

IK_IMPLEMENT(solver_TWO_BONE, solver_base)
{
    IK_OVERRIDE(type_size)
    IK_CONSTRUCTOR(construct)
    IK_DESTRUCTOR(destruct)
    IK_AFTER(rebuild_data)
    IK_AFTER(solve)
}

/*
 * Because we use X macros to fill in the ik interface struct, we have to
 * generate the implementation defines for the node, effector and constraint
 * interfaces as well. These don't actually override anything.
 */
IK_IMPLEMENT(node_TWO_BONE, node_base)
IK_IMPLEMENT(effector_TWO_BONE, effector_base)
IK_IMPLEMENT(constraint_TWO_BONE, constraint_base)

/*
 * Need to combine multiple ik_ret return values from the various before/after
 * functions.
 */
ik_ret ik_solver_TWO_BONE_harness_rebuild_data_return_value(ik_ret a, ik_ret b) {
    if (a != IK_OK) return a;
    return b;
}
ik_ret ik_solver_TWO_BONE_harness_solve_return_value(ik_ret a, ik_ret b) {
    if (a != IK_OK) return a;
    return b;
}
