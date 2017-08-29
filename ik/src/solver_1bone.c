#include "ik/solver.h"
#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/solver_1bone.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
int
solver_1bone_construct(ik_solver_t* solver)
{
    /* set up derived functions */
    solver->destruct = solver_1bone_destruct;
    solver->post_chain_build = solver_1bone_post_chain_build;
    solver->solve = solver_1bone_solve;

    return 0;
}

/* ------------------------------------------------------------------------- */
void
solver_1bone_destruct(ik_solver_t* solver)
{
}

/* ------------------------------------------------------------------------- */
int
solver_1bone_post_chain_build(ik_solver_t* solver)
{
    /*
     * We need to assert that there really are only chains of length 1 and no
     * sub chains.
     */
    SOLVER_FOR_EACH_BASE_CHAIN(solver, base_chain)
        if (chain_length(base_chain) != 2) /* 2 nodes = 1 bone */
        {
            ik_log_message("ERROR: Your tree has chains that are longer than 1 bone. Are you sure you selected the correct solver algorithm?");
            return -1;
        }
        if (chain_length(base_chain) > 0)
        {
            ik_log_message("ERROR: Your tree has child chains. This solver does not support arbitrary trees. You will need to switch to another algorithm (e.g. FABRIK)");
            return -1;
        }
    SOLVER_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
solver_1bone_solve(ik_solver_t* solver)
{
    SOLVER_FOR_EACH_BASE_CHAIN(solver, base_chain)
        ik_node_t* node_tip;
        ik_node_t* node_base;

        assert(chain_length(base_chain) > 1);
        node_tip  = chain_get_node(base_chain, 0);
        node_base = chain_get_node(base_chain, 1);

        assert(node_tip->effector != NULL);
        node_tip->position = node_tip->effector->target_position;

        vec3_sub_vec3(node_tip->position.f, node_base->position.f);
        vec3_normalise(node_tip->position.f);
        vec3_mul_scalar(node_tip->position.f, node_tip->segment_length);
        vec3_add_vec3(node_tip->position.f, node_base->position.f);
    SOLVER_END_EACH

    return 0;
}
