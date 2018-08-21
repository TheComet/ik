#include "ik/ik.h"
#include "ik/memory.h"
#include "ik/log.h"
#include "ik/solver.h"
#include "ik/solver_ONE_BONE.h"
#include "ik/solver_TWO_BONE.h"
#include "ik/solver_FABRIK.h"
#include "ik/solver_MSS.h"
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
uintptr_t
ik_solver_type_size(void)
{
    assert("Calling type_size() on static interface makes no sense.");
    return 0;
}

/* ------------------------------------------------------------------------- */
struct ik_solver_t*
ik_solver_create(enum ik_algorithm_e algorithm)
{
    struct ik_solver_t* solver = NULL;

    switch (algorithm)
    {
#define X(algorithm)                                                          \
        case IK_##algorithm : {                                               \
            solver = MALLOC(ik_solver_##algorithm##_type_size());             \
            if (solver == NULL) {                                             \
                ik_log_fatal("Failed to allocate solver: ran out of memory"); \
                goto alloc_solver_failed;                                     \
            }                                                                 \
            memset(solver, 0, ik_solver_##algorithm##_type_size());           \
            solver->construct = ik_solver_##algorithm##_construct;            \
            solver->destruct = ik_solver_##algorithm##_destruct;              \
            solver->rebuild = ik_solver_##algorithm##_rebuild;                \
            solver->solve = ik_solver_##algorithm##_solve;                    \
        } break;
        IK_ALGORITHMS
#undef X
        default : {
            ik_log_error("Unknown solver algorithm with enum value %d", algorithm);
            goto alloc_solver_failed;
        } break;
    }

    if (solver->construct(solver) != IK_OK)
        goto construct_solver_failed;

    return solver;

    construct_solver_failed : FREE(solver);
    alloc_solver_failed     : return NULL;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destroy(struct ik_solver_t* solver)
{
    solver->destruct(solver);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_construct(struct ik_solver_t* solver)
{
    assert("Calling construct() on static interface makes no sense.");
    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destruct(struct ik_solver_t* solver)
{
    solver->destruct(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_rebuild(struct ik_solver_t* solver)
{
    return solver->rebuild(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_update_distances(struct ik_solver_t* solver)
{
    ik_solver_update_distances(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_solve(struct ik_solver_t* solver)
{
    return ik_solver_solve(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_set_tree(struct ik_solver_t* solver, struct ik_node_t* base)
{
    ik_solver_set_tree(solver, base);
}

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_solver_unlink_tree(struct ik_solver_t* solver)
{
    return ik_solver_unlink_tree(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_iterate_all_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
{
    ik_solver_iterate_all_nodes(solver, callback);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_iterate_affected_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
{
    ik_solver_iterate_affected_nodes(solver, callback);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_iterate_base_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
{
    ik_solver_iterate_base_nodes(solver, callback);
}
