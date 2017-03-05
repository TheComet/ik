#include "ik/solver_FABRIK.h"
#include "ik/node.h"
#include "ik/memory.h"

/* ------------------------------------------------------------------------- */
struct fabrik_t*
solver_FABRIK_create(void)
{
    struct fabrik_t* solver = (struct fabrik_t*)MALLOC(sizeof *solver);
    if(solver == NULL)
        return NULL;

    ordered_vector_construct(&solver->base.fabrik.chain_list);
}

/* ------------------------------------------------------------------------- */
struct fabrik_t*
solver_FABRIK_destroy(struct fabrik_t* solver)
{
    ordered_vector_clear_free(&solver->base.fabrik.chain_list);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
char
solver_FABRIK_solve(struct solver_t* solver)
{

}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

/*!
 * @brief Breaks down the relevant nodes of the scene graph into a list of
 * chains. FABRIK can then more efficiently solve each chain individually.
 *
 * A "sub-base joint" is a node in the scene graph where at least two end
 * effector nodes eventually join together. FABRIK only works on single
 * chains of joints at a time. The end position of every sub-base joint is
 * the average of the resulting multiple positions after running FABRIK on
 * each chain. Said average position becomes the new target position for
 * the next chain connected to it.
 *
 * This algorithm finds all sub-base joints and generates chains between
 * base, sub-base joints, and end effectors. These chains are inserted into
 * the chain list. The order is such that iterating the list from the
 * beginning results in traversing the sub-base nodes breadth-last. This is
 * important.
 *
 * The following constraints must be true in order to avoid malfunctions
 * and segfaults.
 *   + FABRIK assumes all chains have at *least* two nodes. If an IKRoot
 *     and IKEffector component share the same node, or if an IKEffector
 *     happens to be attached to a sub-base node,
 *
 * @note Effectors that are deactivated or invalid are ignored in this search.
 * So even though a node might share two effectors, if one of them is
 * deactivated, then the node is no longer considered a sub-base node.
 */
static char
FABRIK_rebuild_chain_list(struct fabrik_t* solver, struct node_t* root);

/* ------------------------------------------------------------------------- */
static char
FABRIK_rebuild_chain_list(struct fabrik_t* solver, struct node_t* root)
{

}
