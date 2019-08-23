#include "cstructures/memory.h"
#include "ik/algorithm.h"
#include "ik/node_data.h"
#include "ik/log.h"
#include "ik/solver.h"
#include "ik/solver_b1.h"
#include "ik/solver_b2.h"
#include "ik/solver_dummy.h"
#include "ik/solver_fabrik.h"
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_create(struct ik_solver_t** solver,
                 struct ik_algorithm_t* algorithm,
                 struct ik_node_data_t* node_data,
                 uint32_t subbase_idx, uint32_t chain_begin_idx, uint32_t chain_end_idx)
{
    ikret_t status;

    assert(solver != NULL);
    assert(algorithm != NULL);
    assert(node_data != NULL);

    switch (algorithm->type)
    {
#define X(upper, lower)                                                       \
        case IK_SOLVER_##upper : {                                            \
            *solver = MALLOC(sizeof(struct ik_solver_##lower##_t));           \
            if (*solver == NULL) {                                            \
                ik_log_fatal("Failed to allocate solver: ran out of memory"); \
                IK_FAIL(IK_ERR_OUT_OF_MEMORY, alloc_solver_failed);           \
            }                                                                 \
            memset(*solver, 0, sizeof(struct ik_solver_##lower##_t));         \
            (*solver)->init    = (ik_solver_init_func)   ik_solver_##lower##_init; \
            (*solver)->deinit  = (ik_solver_deinit_func) ik_solver_##lower##_deinit; \
            (*solver)->prepare = (ik_solver_prepare_func)ik_solver_##lower##_prepare; \
            (*solver)->solve   = (ik_solver_solve_func)  ik_solver_##lower##_solve; \
        } break;
        IK_SOLVER_ALGORITHM_LIST
#undef X
        default : {
            ik_log_error("Unknown solver solver with enum value %d", solver);
            goto alloc_solver_failed;
        } break;
    }

    IK_INCREF(algorithm);
    (*solver)->algorithm = algorithm;

    if ((status = ik_node_data_view_init(&(*solver)->ndv, node_data, subbase_idx, chain_begin_idx, chain_end_idx)) != IK_OK)
        IK_FAIL(status, init_node_data_view_failed);

    if ((status = (*solver)->init(*solver)) != IK_OK)
        IK_FAIL(status, init_solver_failed);

    return IK_OK;

    init_solver_failed         : ik_node_data_view_deinit(&(*solver)->ndv);
    init_node_data_view_failed : IK_DECREF(algorithm);
                                 FREE(*solver);
    alloc_solver_failed        : return IK_ERR_OUT_OF_MEMORY;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_free(struct ik_solver_t* solver)
{
    solver->deinit(solver);
    ik_node_data_view_deinit(&solver->ndv);
    IK_DECREF(solver->algorithm);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_iterate_nodes(const struct ik_solver_t* solver,
                        ik_solver_callback_func callback)
{
    uint32_t idx = solver->ndv.subbase_idx;
    const struct ik_node_data_t* nda = solver->ndv.node_data;
    callback(nda->user_data[idx], nda->transform[idx].t.position.f, nda->transform[idx].t.rotation.f);
    for (idx = solver->ndv.chain_begin_idx; idx != solver->ndv.chain_end_idx; ++idx)
    callback(nda->user_data[idx], nda->transform[idx].t.position.f, nda->transform[idx].t.rotation.f);
}
