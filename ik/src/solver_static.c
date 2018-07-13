#include "ik/solver_static.h"
#include "ik/ik.h"
#include "ik/memory.h"
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
uintptr_t
ik_solver_static_type_size(void)
{
    assert("Calling type_size() on static interface makes no sense.");
    return 0;
}

/* ------------------------------------------------------------------------- */
struct ik_solver_t*
ik_solver_static_create(enum ik_algorithm_e algorithm)
{
    struct ik_solver_t* solver = NULL;

    switch (algorithm)
    {
#define X(algorithm)                                                          \
        case IK_##algorithm : {                                                                     \
            solver = MALLOC(IKAPI.internal.solver_##algorithm.type_size()); \
            if (solver == NULL) {                                             \
                IKAPI.log.fatal("Failed to allocate solver: ran out of memory"); \
                goto alloc_solver_failed;                                     \
            }                                                                 \
            memset(solver, 0, IKAPI.internal.solver_##algorithm.type_size()); \
            solver->v          = &(IKAPI.internal.solver_##algorithm);  \
            solver->node       = &(IKAPI.internal.node_##algorithm);    \
            solver->effector   = &(IKAPI.internal.effector_##algorithm); \
            solver->constraint = &(IKAPI.internal.constraint_##algorithm); \
            solver->pole       = &(IKAPI.internal.pole_##algorithm); \
        } break;
        IK_ALGORITHMS
#undef X
        default : {
            IKAPI.log.error("Unknown solver algorithm with enum value %d", algorithm);
            goto alloc_solver_failed;
        } break;
    }

    if (solver->v->construct(solver) != IK_OK)
        goto construct_solver_failed;

    return solver;

construct_solver_failed: FREE(solver);
alloc_solver_failed: return NULL;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_destroy(struct ik_solver_t* solver)
{
    solver->v->destruct(solver);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_static_construct(struct ik_solver_t* solver)
{
    assert("Calling construct() on static interface makes no sense.");
    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_destruct(struct ik_solver_t* solver)
{
    solver->v->destruct(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_static_rebuild(struct ik_solver_t* solver)
{
    return solver->v->rebuild(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_update_distances(struct ik_solver_t* solver)
{
    solver->v->update_distances(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_static_solve(struct ik_solver_t* solver)
{
    return solver->v->solve(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_set_tree(struct ik_solver_t* solver, struct ik_node_t* base)
{
    solver->v->set_tree(solver, base);
}

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_solver_static_unlink_tree(struct ik_solver_t* solver)
{
    return solver->v->unlink_tree(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_iterate_all_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
{
    solver->v->iterate_all_nodes(solver, callback);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_iterate_affected_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
{
    solver->v->iterate_affected_nodes(solver, callback);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_iterate_base_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
{
    solver->v->iterate_base_nodes(solver, callback);
}
