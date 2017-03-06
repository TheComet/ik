#include "ik/solver_FABRIK.h"
#include "ik/node.h"
#include "ik/memory.h"
#include "ik/chain.h"
#include "ik/bst_vector.h"
#include "ik/log.h"

struct chain_t
{
    struct ordered_vector_t nodes;  /* list of node_t objects */
    struct vector3_t* base_position;
    struct vector3_t* target_position;
};

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
static int
rebuild_chain_list(struct fabrik_t* solver, struct node_t* root);

/* ------------------------------------------------------------------------- */
struct solver_t*
solver_FABRIK_create(void)
{
    struct fabrik_t* solver = (struct fabrik_t*)MALLOC(sizeof *solver);
    if(solver == NULL)
        return NULL;

    solver->base.solver.private_.destroy = solver_FABRIK_destroy;
    solver->base.solver.private_.solve = solver_FABRIK_solve;

    ordered_vector_construct(&solver->base.fabrik.chain_list, sizeof(struct chain_t));
    return (struct solver_t*)solver;
}

/* ------------------------------------------------------------------------- */
void
solver_FABRIK_destroy(struct solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    ordered_vector_clear_free(&fabrik->base.fabrik.chain_list);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_rebuild_data(struct solver_t* solver)
{
    return -1;
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_solve(struct solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    rebuild_chain_list(fabrik, fabrik->base.solver.private_.tree);

    return -1;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

static int
recursively_mark_involved_nodes(struct solver_t* solver, struct bstv_t* involved_nodes)
{
    /*
     * Traverse the chain of parents starting at each effector node and ending
     * at the root node of the tree and mark every node on the way.
     */
    struct node_t* root = solver->private_.tree;
    ORDERED_VECTOR_FOR_EACH(&solver->private_.effector_nodes_list, struct node_t, effector_node)
        struct node_t* node;
        for(node = effector_node; node != root; node = node->parent)
        {
            if(bstv_insert(involved_nodes, node->guid, NULL) < 0)
            {
                ik_log_message(&solver->log, "Ran out of memory while marking involved nodes");
                return -1;
            }
        }
    ORDERED_VECTOR_END_EACH

    /* mark root node as well */
    if(bstv_insert(involved_nodes, root->guid, NULL) < 0)
    {
        ik_log_message(&solver->log, "Ran out of memory while marking involved nodes");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
rebuild_chain_list(struct fabrik_t* solver, struct node_t* root)
{
    struct bstv_t involved_nodes;
    struct ordered_vector_t* chain_list = &solver->base.fabrik.chain_list;

    bstv_construct(&involved_nodes);
    if(recursively_mark_involved_nodes(solver, &involved_nodes) < 0)
        return -1;

    ordered_vector_clear(chain_list);

    return -1;
}
