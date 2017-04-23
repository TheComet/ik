#include "ik/solver_1bone.h"
#include "ik/chain.h"
#include "ik/log.h"

/* ------------------------------------------------------------------------- */
int
solver_1bone_construct(ik_solver_t* solver)
{
    one_bone_t* one_bone = (one_bone_t*)solver;

    /* set up derived functions */
    one_bone->destruct = solver_1bone_destruct;
    one_bone->rebuild_data = solver_1bone_rebuild;
    one_bone->solve = solver_1bone_solve;

    return 0;
}

/* ------------------------------------------------------------------------- */
void
solver_1bone_destruct(ik_solver_t* solver)
{
}

/* ------------------------------------------------------------------------- */
int
solver_1bone_rebuild(ik_solver_t* solver)
{
    /*
     * We need to assert that there really are only chains of length 1 and no
     * sub chains.
     */
    ORDERED_VECTOR_FOR_EACH(&solver->chain_tree->children, ik_chain_t, child)
        if (ordered_vector_count(&child->nodes) > 2) /* 2 nodes = 1 bone */
        {
            ik_log_message("WARNING: Your tree has chains that are longer than 1 bone. Are you sure you selected the correct solver algorithm?");
            return -1;
        }
        if (ordered_vector_count(&child->children) > 0)
        {
            ik_log_message("WARNING: Your tree has child chains. This solver does not support arbitrary trees. You will need to switch to another algorithm (e.g. FABRIK)");
            return -1;
        }
    ORDERED_VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
solver_1bone_solve(ik_solver_t* solver)
{
    return 0;
}
