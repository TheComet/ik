#include "ik/log.h"
#include "ik/solver.h"
#include "ik/solver_group.h"
#include <assert.h>
#include <string.h>

struct ik_solver_group
{
    IK_SOLVER_HEAD

    struct vector_t solver_list;  /* list of ik_solver* */
};

/* ------------------------------------------------------------------------- */
static int
solver_group_init(struct ik_solver* solver, const struct ik_subtree* subtree)
{
    ik_log_printf(IK_ERROR, "solver_group init() called! This should not happen ever!");
    return -1;
}

/* ------------------------------------------------------------------------- */
static void
solver_group_deinit(struct ik_solver* solver)
{
    struct ik_solver_group* group = (struct ik_solver_group*)solver;

    VECTOR_FOR_EACH(&group->solver_list, struct ik_solver*, p_solver)
        IK_DECREF(*p_solver);
    VECTOR_END_EACH
    vector_deinit(&group->solver_list);
}

/* ------------------------------------------------------------------------- */
static void
solver_group_update_translations(struct ik_solver* solver)
{
    struct ik_solver_group* group = (struct ik_solver_group*)solver;

    VECTOR_FOR_EACH(&group->solver_list, struct ik_solver*, child_solver)
        (*child_solver)->impl.update_translations(*child_solver);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static int
solver_group_solve(struct ik_solver* solver)
{
    struct ik_solver_group* group = (struct ik_solver_group*)solver;
    int result = 0;

    VECTOR_FOR_EACH(&group->solver_list, struct ik_solver*, child_solver)
        result += group->impl.solve(*child_solver);
    VECTOR_END_EACH

    return result;
}

/* ------------------------------------------------------------------------- */
static void
solver_group_iterate_nodes(const struct ik_solver* solver, ik_solver_callback_func cb)
{
    const struct ik_solver_group* group = (const struct ik_solver_group*)solver;

    VECTOR_FOR_EACH(&group->solver_list, struct ik_solver*, child_solver)
        solver->impl.iterate_nodes(*child_solver, cb);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_group = {
    "solver group",
    sizeof(struct ik_solver_group),
    solver_group_init,
    solver_group_deinit,
    solver_group_update_translations,
    solver_group_solve,
    solver_group_iterate_nodes
};

/* ------------------------------------------------------------------------- */
struct ik_solver*
ik_solver_group_create(struct vector_t solver_list)
{
    struct ik_solver_group* solver_group = (struct ik_solver_group*)
        ik_refcounted_alloc(sizeof *solver_group, (ik_deinit_func)solver_group_deinit);
    if (solver_group == NULL)
        return NULL;

    solver_group->impl = ik_solver_group;
    solver_group->algorithm = NULL;
    solver_group->solver_list = solver_list;

    VECTOR_FOR_EACH(&solver_group->solver_list, struct ik_solver*, child_solver)
        IK_INCREF(*child_solver);
    VECTOR_END_EACH

    return (struct ik_solver*)solver_group;
}
