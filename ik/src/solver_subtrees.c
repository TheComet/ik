#include "ik/log.h"
#include "ik/solver.h"
#include "ik/solver_subtrees.h"
#include <assert.h>
#include <string.h>

struct ik_solver_subtrees
{
    IK_SOLVER_HEAD

    struct cs_vector solver_list;  /* list of ik_solver* */
};

/* ------------------------------------------------------------------------- */
static int
solver_subtrees_init(struct ik_solver* solver, const struct ik_subtree* subtree)
{
    ik_log_printf(IK_ERROR, "solver_subtrees init() called! This should not happen ever!");
    return -1;
}

/* ------------------------------------------------------------------------- */
static void
solver_subtrees_deinit(struct ik_solver* solver)
{
    struct ik_solver_subtrees* subtrees = (struct ik_solver_subtrees*)solver;

    VECTOR_FOR_EACH(&subtrees->solver_list, struct ik_solver*, p_solver)
        IK_DECREF(*p_solver);
    VECTOR_END_EACH
    vector_deinit(&subtrees->solver_list);
}

/* ------------------------------------------------------------------------- */
static void
solver_subtrees_solve(struct ik_solver* solver)
{
    struct ik_solver_subtrees* subtrees = (struct ik_solver_subtrees*)solver;

    VECTOR_FOR_EACH(&subtrees->solver_list, struct ik_solver*, child_solver)
        subtrees->impl.solve(*child_solver);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
solver_subtrees_iterate_nodes(const struct ik_solver* solver, ik_solver_callback_func cb, int skip_base)
{
    unsigned i;
    struct ik_solver** child;
    const struct ik_solver_subtrees* subtrees = (const struct ik_solver_subtrees*)solver;

    /* Last solver in subtrees is the root-most solver, and is the only solver that
     * references the base node. We pass skip_base to it */
    i = vector_count(&subtrees->solver_list) - 1;
    child = vector_get_element(&subtrees->solver_list, i);
    (*child)->impl.iterate_nodes(*child, cb, skip_base);

    /* Every other solver doesn't reference the base node so explicitely call
     * with skip_base=0 */
    while (i-- > 0)
    {
        child = vector_get_element(&subtrees->solver_list, i);
        (*child)->impl.iterate_nodes(*child, cb, 0);
    }
}

/* ------------------------------------------------------------------------- */
static void
solver_subtrees_iterate_effector_nodes(const struct ik_solver* solver, ik_solver_callback_func cb)
{
    const struct ik_solver_subtrees* subtrees = (const struct ik_solver_subtrees*)solver;

    VECTOR_FOR_EACH(&subtrees->solver_list, const struct ik_solver*, child)
        (*child)->impl.iterate_effector_nodes(*child, cb);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_subtrees = {
    "subtrees",
    sizeof(struct ik_solver_subtrees),
    solver_subtrees_init,
    solver_subtrees_deinit,
    solver_subtrees_solve,
    solver_subtrees_iterate_nodes,
    solver_subtrees_iterate_effector_nodes
};

/* ------------------------------------------------------------------------- */
struct ik_solver*
ik_solver_subtrees_create(struct cs_vector solver_list)
{
    struct ik_solver_subtrees* subtrees = (struct ik_solver_subtrees*)
        ik_refcounted_alloc(sizeof *subtrees, (ik_deinit_func)solver_subtrees_deinit);
    if (subtrees == NULL)
        return NULL;

    subtrees->impl = ik_solver_subtrees;
    subtrees->algorithm = NULL;
    subtrees->solver_list = solver_list;

    VECTOR_FOR_EACH(&subtrees->solver_list, struct ik_solver*, child_solver)
        IK_INCREF(*child_solver);
    VECTOR_END_EACH

    return (struct ik_solver*)subtrees;
}
