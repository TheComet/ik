#include "ik/solver.h"
#include "ik/solver_FABRIK.h"
#include "ik/solver_jacobian_inverse.h"
#include "ik/solver_jacobian_transpose.h"
#include "ik/node.h"
#include "ik/memory.h"

/* ------------------------------------------------------------------------- */
struct solver_t*
ik_solver_create(enum algorithm_e algorithm)
{
    struct solver_t* solver = NULL;

    switch(algorithm)
    {
    case ALGORITHM_FABRIK:
        solver = (struct solver_t*)solver_FABRIK_create();
        solver->base.solver.destroy = solver_FABRIK_destroy;
        solver->base.solver.solve = solver_FABRIK_solve;
        break;

    case ALGORITHM_JACOBIAN_INVERSE:
    case ALGORITHM_JACOBIAN_TRANSPOSE:
        /* not implemented yet */
        break;
    }

    return solver;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_destroy(struct solver_t* solver)
{
    solver->base.solver.destroy(solver);
}

/* ------------------------------------------------------------------------- */
struct node_t*
ik_solver_create_tree(struct solver_t* solver, uint32_t guid)
{
    if(solver->base.solver.tree != NULL)
        node_destroy(solver->base.solver.tree);

    solver->base.solver.tree = node_create(guid);
    return solver->base.solver.tree;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_solve(struct solver_t* solver)
{
    return solver->base.solver.solve(solver);
}
