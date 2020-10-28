#include "ik/log.h"
#include "ik/solver.h"
#include "ik/meta_solvers.h"
#include "cstructures/memory.h"
#include <assert.h>
#include <string.h>

struct ik_solver_group
{
    IK_SOLVER_HEAD

    struct ik_solver** subsolvers;
    int subsolver_count;
};

/* ------------------------------------------------------------------------- */
static void
solver_group_deinit(struct ik_solver* solver_base)
{
    int i;
    struct ik_solver_group* solver = (struct ik_solver_group*)solver_base;

    for (i = 0; i != solver->subsolver_count; ++i)
        IK_DECREF(solver->subsolvers[i]);

    FREE(solver->subsolvers);
}

/* ------------------------------------------------------------------------- */
static int
solver_group_solve(struct ik_solver* solver_base)
{
    int i;
    struct ik_solver_group* solver = (struct ik_solver_group*)solver_base;

    int iterations = 0;
    for (i = 0; i != solver->subsolver_count; ++i)
    {
        struct ik_solver* subsolver = solver->subsolvers[i];
        iterations += subsolver->impl.solve(subsolver);
    }

    return iterations;
}

/* ------------------------------------------------------------------------- */
static void
solver_group_visit_bones(const struct ik_solver* solver_base, ik_visit_bone_func visit, void* param)
{
    int i;
    struct ik_solver_group* solver = (struct ik_solver_group*)solver_base;

    for (i = 0; i != solver->subsolver_count; ++i)
        solver->subsolvers[i]->impl.visit_bones(solver->subsolvers[i], visit, param);
}

/* ------------------------------------------------------------------------- */
static void
solver_group_visit_effectors(const struct ik_solver* solver_base, ik_visit_bone_func visit, void* param)
{
    int i;
    struct ik_solver_group* solver = (struct ik_solver_group*)solver_base;

    for (i = 0; i != solver->subsolver_count; ++i)
        solver->subsolvers[i]->impl.visit_effectors(solver->subsolvers[i], visit, param);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_group = {
    "group",
    sizeof(struct ik_solver_group),
    NULL,
    solver_group_deinit,
    solver_group_solve,
    solver_group_visit_bones,
    solver_group_visit_effectors
};

/* ------------------------------------------------------------------------- */
struct ik_solver*
ik_solver_group_create(const struct cs_vector* solver_list)
{
    cs_vec_idx i;

    struct ik_solver_group* solver = (struct ik_solver_group*)
        ik_solver_alloc(&ik_solver_group, NULL, NULL);
    if (solver == NULL)
        goto alloc_solver_failed;

    solver->subsolver_count = vector_count(solver_list);
    solver->subsolvers = MALLOC(solver->subsolver_count * sizeof(*solver->subsolvers));
    if (solver->subsolvers == NULL)
        goto alloc_subsolvers_failed;

    for (i = 0; i != solver->subsolver_count; ++i)
    {
        struct ik_solver** psubsolver = vector_get_element(solver_list, i);
        IK_INCREF(*psubsolver);
        solver->subsolvers[i] = *psubsolver;
    }

    ik_log_printf(IK_DEBUG, "Group: Initialized with %d isolated subsolvers",
                  vector_count(solver_list));

    return (struct ik_solver*)solver;

    alloc_subsolvers_failed : ik_solver_free((struct ik_solver*)solver);
    alloc_solver_failed     : return NULL;
}
