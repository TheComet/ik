#include "ik/log.h"
#include "ik/solver.h"
#include "ik/meta_solvers.h"
#include <assert.h>
#include <string.h>

struct subsolver
{
    struct ik_solver* solver;
};

struct ik_solver_group
{
    IK_SOLVER_HEAD

    struct cs_vector subsolvers;  /* list of struct subsolver */
};

/* ------------------------------------------------------------------------- */
static int
solver_group_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    ik_log_printf(IK_ERROR, "solver_group_init() called! This should not happen ever!");
    assert(0);
    return -1;
}

/* ------------------------------------------------------------------------- */
static void
solver_group_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_group* solver = (struct ik_solver_group*)solver_base;

    VECTOR_FOR_EACH(&solver->subsolvers, struct subsolver, subsolver)
        IK_DECREF(subsolver->solver);
    VECTOR_END_EACH
    vector_deinit(&solver->subsolvers);
}

/* ------------------------------------------------------------------------- */
static int
solver_group_solve(struct ik_solver* solver_base)
{
    struct ik_solver_group* solver = (struct ik_solver_group*)solver_base;

    int iterations = 0;
    VECTOR_FOR_EACH(&solver->subsolvers, struct subsolver, subsolver)
        iterations += subsolver->solver->impl.solve(subsolver->solver);
    VECTOR_END_EACH

    return iterations;
}

/* ------------------------------------------------------------------------- */
static void
solver_group_visit_nodes(const struct ik_solver* solver_base, ik_visit_node_func visit, void* param, int skip_base)
{
    unsigned i;
    struct ik_solver** child;
    const struct ik_solver_group* solver = (const struct ik_solver_group*)solver_base;

    /* Last solver in list is the root-most solver, and is the only solver that
     * references the base node. We pass skip_base to it */
    i = vector_count(&solver->subsolvers) - 1;
    child = vector_get_element(&solver->subsolvers, i);
    (*child)->impl.visit_nodes(*child, visit, param, skip_base);

    /* Every other solver doesn't reference the base node so explicitely call
     * with skip_base=0 */
    while (i-- > 0)
    {
        child = vector_get_element(&solver->subsolvers, i);
        (*child)->impl.visit_nodes(*child, visit, param, 0);
    }
}

/* ------------------------------------------------------------------------- */
static void
solver_group_visit_effector_nodes(const struct ik_solver* solver_base, ik_visit_node_func visit, void* param)
{
    const struct ik_solver_group* solver = (const struct ik_solver_group*)solver_base;

    VECTOR_FOR_EACH(&solver->subsolvers, struct subsolver, subsolver)
        subsolver->solver->impl.visit_effector_nodes(subsolver->solver, visit, param);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
solver_group_get_first_segment(const struct ik_solver* solver_base, struct ik_node** base, struct ik_node** tip)
{
    ik_log_printf(IK_ERROR, "solver_group_get_first_segment() called! This should not happen ever!");
    assert(0);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_group = {
    "group",
    sizeof(struct ik_solver_group),
    solver_group_init,
    solver_group_deinit,
    solver_group_solve,
    solver_group_visit_nodes,
    solver_group_visit_effector_nodes,
    solver_group_get_first_segment
};

/* ------------------------------------------------------------------------- */
struct ik_solver*
ik_solver_group_create(const struct cs_vector* solver_list)
{
    struct ik_solver_group* solver = (struct ik_solver_group*)
        ik_refcounted_alloc(sizeof *solver, (ik_deinit_func)solver_group_deinit);
    if (solver == NULL)
        goto alloc_solver_failed;

    solver->impl = ik_solver_group;
    solver->algorithm = NULL;

    vector_init(&solver->subsolvers, sizeof(struct subsolver));
    VECTOR_FOR_EACH(solver_list, struct ik_solver*, subsolver)
        struct subsolver* data = vector_emplace(&solver->subsolvers);
        if (data == NULL)
            goto add_subsolver_failed;

        IK_INCREF(*subsolver);
        data->solver = *subsolver;
    VECTOR_END_EACH

    ik_log_printf(IK_DEBUG, "Group: Initialized with %d isolated subsolvers",
                  vector_count(solver_list));

    return (struct ik_solver*)solver;

    add_subsolver_failed : VECTOR_FOR_EACH(&solver->subsolvers, struct subsolver, subsolver)
                               IK_DECREF(subsolver->solver);
                           VECTOR_END_EACH
                           vector_deinit(&solver->subsolvers);
                           ik_refcounted_obj_free((struct ik_refcounted*)solver);
    alloc_solver_failed  : return NULL;
}
