#include "ik/solver_ONE_BONE.h"
#include "ik/ik.h"
#include "ik/chain.h"
#include "ik/vec3_static.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
uintptr_t
ik_solver_ONE_BONE_type_size(void)
{
    return sizeof(struct ik_solver_t);
}

/* ------------------------------------------------------------------------- */
int
ik_solver_ONE_BONE_construct(struct ik_solver_t* solver)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_ONE_BONE_destruct(struct ik_solver_t* solver)
{
}

/* ------------------------------------------------------------------------- */
int
ik_solver_ONE_BONE_rebuild(struct ik_solver_t* solver)
{
    /*
     * We need to assert that there really are only chains of length 1 and no
     * sub chains.
     */
    SOLVER_FOR_EACH_CHAIN(solver, chain)
        if (chain_length(chain) != 2) /* 2 nodes = 1 bone */
        {
            IKAPI.log.message("ERROR: Your tree has chains that are longer than 1 bone. Are you sure you selected the correct solver algorithm?");
            return -1;
        }
        if (chain_length(chain) > 0)
        {
            IKAPI.log.message("ERROR: Your tree has child chains. This solver does not support arbitrary trees. You will need to switch to another algorithm (e.g. FABRIK)");
            return -1;
        }
    SOLVER_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_ONE_BONE_solve(struct ik_solver_t* solver)
{
    SOLVER_FOR_EACH_CHAIN(solver, chain)
        struct ik_node_t* node_tip;
        struct ik_node_t* node_base;

        assert(chain_length(chain) > 1);
        node_tip  = chain_get_node(chain, 0);
        node_base = chain_get_node(chain, 1);

        assert(node_tip->effector != NULL);
        node_tip->position = node_tip->effector->target_position;

        ik_vec3_static_sub_vec3(node_tip->position.f, node_base->position.f);
        ik_vec3_static_normalize(node_tip->position.f);
        ik_vec3_static_mul_scalar(node_tip->position.f, node_tip->dist_to_parent);
        ik_vec3_static_add_vec3(node_tip->position.f, node_base->position.f);
    SOLVER_END_EACH

    return 0;
}
