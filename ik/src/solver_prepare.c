#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/ntf.h"
#include "ik/node_data.h"
#include "ik/solver_prepare.h"
#include "ik/solver.h"
#include "ik/transform.h"
#include "ik/vector.h"
#include <stddef.h>
#include <assert.h>

struct ik_solver_t
{
    IK_SOLVER_HEAD
};

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_prepare_stack_buffer(struct ik_solver_t* solver)
{
    /*
     * The solver needs a small stack to push/pop transformations as it
     * iterates the tree.
     * TODO: Add support for alloca(). If the stack is small enough and the
     * platform supports alloca(), leave this as NULL.
     */
    uint32_t max_children = 0;
    NTF_FOR_EACH(&solver->ntf_list, ntf)
        uint32_t candidate = ik_ntf_find_highest_child_count(ntf);
        if (max_children < candidate)
            max_children = candidate;
    NTF_END_EACH

    XFREE(solver->stack_buffer);
    solver->stack_buffer = NULL;

    /* Simple trees don't have more than 1 child */
    if (max_children <= 1)
        return IK_OK;

    solver->stack_buffer = MALLOC(sizeof(union ik_transform_t) * max_children);
    if (solver->stack_buffer == NULL)
    {
        ik_log_fatal("Failed to allocate solver stack: Ran out of memory");
        return IK_ERR_OUT_OF_MEMORY;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_prepare_effector_chains(struct ik_solver_t* solver)
{
    ikret_t status;
    vector_clear_free(&solver->effector_chains);

    NTF_FOR_EACH(&solver->ntf_list, ntf)
        uint32_t i = ntf->node_count;
        struct ik_chain_t chain = {0};
        while (i--)
        {
            /* Found an effector node */
            if (ntf->indices[i].post_child_count == 0)
                chain.effector_node = &ntf->node_data[ntf->indices[i].post];

            if (ntf->indices[i].post_child_count > 1)
            {
                chain.base_node = &ntf->node_data[ntf->indices[i].post];
                assert(chain.effector_node != NULL);
                assert(chain.effector_node->effector != NULL);
                if ((status = vector_push(&solver->effector_chains, &chain)) != IK_OK)
                    return status;

                chain.effector_node = NULL;  /* To trip assert() if the ntf is somehow wrong */
            }
        }
    NTF_END_EACH

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_prepare_pole_targets(struct ik_solver_t* solver)
{
    NTF_FOR_EACH(&solver->ntf_list, ntf)
    NTF_END_EACH
}
