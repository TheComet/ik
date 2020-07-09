#include "ik/log.h"
#include "ik/solver.h"
#include "cstructures/vector.h"
#include <assert.h>
#include <string.h>

struct ik_solver_combine
{
    IK_SOLVER_HEAD

    struct cs_vector solvers;  /* list of ik_solver* */
};

/* ------------------------------------------------------------------------- */
static int
solver_combine_init(struct ik_solver* solver, const struct ik_subtree* subtree)
{
    ik_log_printf(IK_ERROR, "solver_combine_init() called! This should not happen ever!");
    return -1;
}

/* ------------------------------------------------------------------------- */
static void
solver_combine_deinit(struct ik_solver* solver)
{
    struct ik_solver_combine* list = (struct ik_solver_combine*)solver;

    VECTOR_FOR_EACH(&list->solvers, struct ik_solver*, p_solver)
        IK_DECREF(*p_solver);
    VECTOR_END_EACH
    vector_deinit(&list->solvers);
}

/* ------------------------------------------------------------------------- */
static int
solver_combine_solve(struct ik_solver* solver)
{
    struct ik_solver_combine* list = (struct ik_solver_combine*)solver;

    int iterations = 0;
    VECTOR_FOR_EACH(&list->solvers, struct ik_solver*, child_solver)
        iterations += list->impl.solve(*child_solver);
    VECTOR_END_EACH

    return iterations;
}

/* ------------------------------------------------------------------------- */
static void
solver_combine_iterate_nodes(const struct ik_solver* solver, ik_solver_callback_func cb, int skip_base)
{
    unsigned i;
    struct ik_solver** child;
    const struct ik_solver_combine* list = (const struct ik_solver_combine*)solver;

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
solver_combine_iterate_effector_nodes(const struct ik_solver* solver, ik_solver_callback_func cb)
{
    const struct ik_solver_combine* list = (const struct ik_solver_combine*)solver;

    VECTOR_FOR_EACH(&list->solvers, const struct ik_solver*, child)
        (*child)->impl.iterate_effector_nodes(*child, cb);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_combine = {
    "combine",
    sizeof(struct ik_solver_combine),
    solver_combine_init,
    solver_combine_deinit,
    solver_combine_solve,
    solver_combine_iterate_nodes,
    solver_combine_iterate_effector_nodes
};

/* ------------------------------------------------------------------------- */
struct ik_solver*
ik_solver_combine_create(struct cs_vector solver_combine)
{
    struct ik_solver_combine* list = (struct ik_solver_combine*)
        ik_refcounted_alloc(sizeof *list, (ik_deinit_func)solver_combine_deinit);
    if (list == NULL)
        return NULL;

    list->impl = ik_solver_combine;
    list->algorithm = NULL;
    list->solvers = solver_combine;

    VECTOR_FOR_EACH(&list->solvers, struct ik_solver*, child_solver)
        IK_INCREF(*child_solver);
    VECTOR_END_EACH

    return (struct ik_solver*)list;
}
