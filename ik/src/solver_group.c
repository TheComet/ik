#include "ik/log.h"
#include "ik/solver.h"
#include "ik/meta_solvers.h"
#include <assert.h>
#include <string.h>

struct ik_solver_group
{
    IK_SOLVER_HEAD

    struct cs_vector solvers;  /* list of ik_solver* */
};

/* ------------------------------------------------------------------------- */
static int
solver_group_init(struct ik_solver* solver, const struct ik_subtree* subtree)
{
    ik_log_printf(IK_ERROR, "solver_group_init() called! This should not happen ever!");
    return -1;
}

/* ------------------------------------------------------------------------- */
static void
solver_group_deinit(struct ik_solver* solver)
{
    struct ik_solver_group* list = (struct ik_solver_group*)solver;

    VECTOR_FOR_EACH(&list->solvers, struct ik_solver*, p_solver)
        IK_DECREF(*p_solver);
    VECTOR_END_EACH
    vector_deinit(&list->solvers);
}

/* ------------------------------------------------------------------------- */
static int
solver_group_solve(struct ik_solver* solver)
{
    struct ik_solver_group* list = (struct ik_solver_group*)solver;

    int iterations = 0;
    VECTOR_FOR_EACH(&list->solvers, struct ik_solver*, child_solver)
        iterations += list->impl.solve(*child_solver);
    VECTOR_END_EACH

    return iterations;
}

/* ------------------------------------------------------------------------- */
static void
solver_group_iterate_nodes(const struct ik_solver* solver, ik_solver_callback_func cb, int skip_base)
{
    unsigned i;
    struct ik_solver** child;
    const struct ik_solver_group* list = (const struct ik_solver_group*)solver;

    /* Last solver in list is the root-most solver, and is the only solver that
     * references the base node. We pass skip_base to it */
    i = vector_count(&list->solvers) - 1;
    child = vector_get_element(&list->solvers, i);
    (*child)->impl.iterate_nodes(*child, cb, skip_base);

    /* Every other solver doesn't reference the base node so explicitely call
     * with skip_base=0 */
    while (i-- > 0)
    {
        child = vector_get_element(&list->solvers, i);
        (*child)->impl.iterate_nodes(*child, cb, 0);
    }
}

/* ------------------------------------------------------------------------- */
static void
solver_group_iterate_effector_nodes(const struct ik_solver* solver, ik_solver_callback_func cb)
{
    const struct ik_solver_group* list = (const struct ik_solver_group*)solver;

    VECTOR_FOR_EACH(&list->solvers, const struct ik_solver*, child)
        (*child)->impl.iterate_effector_nodes(*child, cb);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_group = {
    "group",
    sizeof(struct ik_solver_group),
    solver_group_init,
    solver_group_deinit,
    solver_group_solve,
    solver_group_iterate_nodes,
    solver_group_iterate_effector_nodes
};

/* ------------------------------------------------------------------------- */
struct ik_solver*
ik_solver_group_create(struct cs_vector solver_group)
{
    struct ik_solver_group* list = (struct ik_solver_group*)
        ik_refcounted_alloc(sizeof *list, (ik_deinit_func)solver_group_deinit);
    if (list == NULL)
        return NULL;

    list->impl = ik_solver_group;
    list->algorithm = NULL;
    list->solvers = solver_group;

    VECTOR_FOR_EACH(&solver_group, struct ik_solver*, psolver)
        IK_INCREF(*psolver);
    VECTOR_END_EACH

    return (struct ik_solver*)list;
}
