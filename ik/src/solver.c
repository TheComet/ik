#include "ik/log.h"
#include "ik/memory.h"
#include "ik/ntf.h"
#include "ik/solver_head.h"
#include "ik/solver.h"
#include "ik/solver_prepare.h"
#include "ik/solver_update.h"
#include "ik/solver_ONE_BONE.h"
#include "ik/solver_TWO_BONE.h"
#include "ik/solver_FABRIK.h"
#include "ik/solver_MSS.h"
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_create(struct ik_solver_t** solver, enum ik_solver_algorithm_e algorithm)
{
    switch (algorithm)
    {
#define X(algorithm)                                                          \
        case IK_SOLVER_##algorithm : {                                        \
            *solver = MALLOC(ik_solver_##algorithm##_type_size());            \
            if (*solver == NULL) {                                            \
                ik_log_fatal("Failed to allocate solver: ran out of memory"); \
                goto alloc_solver_failed;                                     \
            }                                                                 \
            memset(*solver, 0, ik_solver_##algorithm##_type_size());          \
            (*solver)->construct = ik_solver_##algorithm##_construct;         \
            (*solver)->destruct = ik_solver_##algorithm##_destruct;           \
            (*solver)->prepare = ik_solver_##algorithm##_prepare;             \
            (*solver)->solve = ik_solver_##algorithm##_solve;                 \
        } break;
        IK_SOLVER_ALGORITHM_LIST
#undef X
        default : {
            ik_log_error("Unknown solver algorithm with enum value %d", algorithm);
            goto alloc_solver_failed;
        } break;
    }

    if (ik_solver_construct(*solver) != IK_OK)
        goto construct_solver_failed;

    return IK_OK;

    construct_solver_failed : FREE(*solver);
    alloc_solver_failed     : return IK_ERR_OUT_OF_MEMORY;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destroy(struct ik_solver_t* solver)
{
    ik_solver_destruct(solver);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_construct(struct ik_solver_t* solver)
{
    solver->max_iterations = 20;
    solver->tolerance = 1e-2;
    solver->features = IK_SOLVER_JOINT_ROTATIONS;
    ik_ntf_list_construct(&solver->ntf_list);

    return solver->construct(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destruct(struct ik_solver_t* solver)
{
    solver->destruct(solver);
    ik_ntf_list_clear(&solver->ntf_list);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_prepare(struct ik_solver_t* solver, struct ik_node_t* node)
{
    ikret_t status;

    if ((status = ik_ntf_list_fill(&solver->ntf_list, node)) != IK_OK)
        return status;

    if ((status = ik_solver_prepare_stack_buffer(solver)) != IK_OK)
        return status;

    ik_solver_prepare_pole_targets(solver);
    ik_solver_update_node_distances(&solver->ntf_list);

    return solver->prepare(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_update_translations(struct ik_solver_t* solver)
{
    ik_solver_update_node_distances(&solver->ntf_list);
}

/* ------------------------------------------------------------------------- */
uint32_t
ik_solver_solve(struct ik_solver_t* solver)
{
    ik_solver_update_effector_targets(&solver->ntf_list);
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
ik_solver_enable_features(struct ik_solver_t* solver, uint16_t features)
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

