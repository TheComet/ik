#include "ik/log.h"
#include "ik/memory.h"
#include "ik/ntf.h"
#include "ik/solver_head.h"
#include "ik/solver.h"
#include "ik/solver_prepare.h"
#include "ik/solver_update.h"
#include "ik/solver_b1.h"
#include "ik/solver_b2.h"
#include "ik/solver_fabrik.h"
#include "ik/solver_mss.h"
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_create(struct ik_solver_t** solver, enum ik_solver_type type)
{
    switch (type)
    {
#define X(upper, lower)                                                       \
        case IK_ALGORITHM_##upper : {                                         \
            *solver = MALLOC(ik_solver_##lower##_type_size());          \
            if (*solver == NULL) {                                         \
                ik_log_fatal("Failed to allocate solver: ran out of memory"); \
                goto alloc_solver_failed;                                  \
            }                                                                 \
            memset(*solver, 0, ik_solver_##lower##_type_size());        \
            (*solver)->init = (ik_solver_init_func) ik_solver_##lower##_init; \
            (*solver)->deinit  = (ik_solver_deinit_func)  ik_solver_##lower##_deinit; \
            (*solver)->prepare   = (ik_solver_prepare_func)   ik_solver_##lower##_prepare; \
            (*solver)->solve     = (ik_solver_solve_func)     ik_solver_##lower##_solve; \
        } break;
        IK_ALGORITHM_LIST
#undef X
        default : {
            ik_log_error("Unknown solver solver with enum value %d", solver);
            goto alloc_solver_failed;
        } break;
    }

    if (ik_solver_init(*solver) != IK_OK)
        goto init_solver_failed;

    return IK_OK;

    init_solver_failed : FREE(*solver);
    alloc_solver_failed     : return IK_ERR_OUT_OF_MEMORY;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_free(struct ik_solver_t* solver)
{
    ik_solver_deinit(solver);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_init(struct ik_solver_t* solver)
{
    solver->max_iterations = 20;
    solver->tolerance = 1e-2;
    solver->features = IK_ALGORITHM_JOINT_ROTATIONS;

    return solver->init(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_deinit(struct ik_solver_t* solver)
{
    solver->deinit(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_prepare(struct ik_solver_t* solver, struct ik_packed_tree_t* ntf)
{
    ikret_t status;

    if ((status = ik_solver_prepare_stack_buffer(solver)) != IK_OK)
        return status;

    ik_solver_prepare_pole_targets(solver);
    ik_solver_update_node_distances(solver);

    return solver->prepare(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_update_translations(struct ik_solver_t* solver)
{
    ik_solver_update_node_distances(solver);
}

/* ------------------------------------------------------------------------- */
uint32_t
ik_solver_solve(struct ik_solver_t* solver)
{
    ik_solver_update_effector_targets(solver);
    return solver->solve(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_iterate_nodes(const struct ik_solver_t* solver,
                        ik_solver_callback_func callback)
{
}


/* ------------------------------------------------------------------------- */
uint16_t
ik_solver_get_max_iterations(const struct ik_solver_t* solver)
{
    return solver->max_iterations;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_set_max_iterations(struct ik_solver_t* solver, uint16_t max_iterations)
{
    solver->max_iterations = max_iterations;
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_solver_get_tolerance(const struct ik_solver_t* solver)
{
    return solver->tolerance;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_set_tolerance(struct ik_solver_t* solver, ikreal_t tolerance)
{
    solver->tolerance = tolerance;
}

/* ------------------------------------------------------------------------- */
uint16_t
ik_solver_get_features(const struct ik_solver_t* solver)
{
    return solver->features;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_typenable_features(struct ik_solver_t* solver, uint16_t features)
{
    solver->features |= features;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_disable_features(struct ik_solver_t* solver, uint16_t features)
{
    solver->features &= ~features;
}

/* ------------------------------------------------------------------------- */
uint8_t
ik_solver_is_feature_enabled(const struct ik_solver_t* solver,
                             enum ik_solver_features_e feature)
{
    return (solver->features & feature) == feature;
}

