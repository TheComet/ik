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
        case IK_##algorithm:                                                  \
            solver = MALLOC(ik.internal.solver_##algorithm.type_size());      \
            if (solver == NULL) {                                             \
                ik.log.message("Failed to allocate solver: ran out of memory"); \
                goto alloc_solver_failed;                                     \
            }                                                                 \
            memset(solver, 0, ik.internal.solver_##algorithm.type_size());    \
            solver->v          = &(ik.internal.solver_##algorithm);           \
            solver->node       = &(ik.internal.node_##algorithm);             \
            solver->effector   = &(ik.internal.effector_##algorithm);         \
            solver->constraint = &(ik.internal.constraint_##algorithm);       \
            break;
        IK_ALGORITHMS
#undef X
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
    return solver->v->construct(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_destruct(struct ik_solver_t* solver)
{
    solver->v->destruct(solver);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_static_rebuild_data(struct ik_solver_t* solver)
{
    return solver->v->rebuild_data(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_recalculate_segment_lengths(struct ik_solver_t* solver)
{
    solver->v->recalculate_segment_lengths(solver);
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
ik_solver_static_destroy_tree(struct ik_solver_t* solver)
{
    solver->v->destroy_tree(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_static_iterate_nodes(struct ik_solver_t* solver, ik_solver_iterate_node_cb_func callback)
{
    solver->v->iterate_nodes(solver, callback);
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
