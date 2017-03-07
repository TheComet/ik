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
    struct vector3_t* end_position;
};

void
clear_chain_list(struct ordered_vector_t* chain_list);

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
rebuild_chain_list(struct fabrik_t* solver);

/* ------------------------------------------------------------------------- */
struct solver_t*
solver_FABRIK_create(void)
{
    struct fabrik_t* solver = (struct fabrik_t*)MALLOC(sizeof *solver);
    if(solver == NULL)
        return NULL;

    solver->base.solver.private_.destroy = solver_FABRIK_destroy;
    solver->base.solver.private_.rebuild_data = solver_FABRIK_rebuild_data;
    solver->base.solver.private_.solve = solver_FABRIK_solve;

    ordered_vector_construct(&solver->base.fabrik.chain_list, sizeof(struct chain_t));
    return (struct solver_t*)solver;
}

/* ------------------------------------------------------------------------- */
void
solver_FABRIK_destroy(struct solver_t* solver)
{
    struct fabrik_t* fabrik = (struct fabrik_t*)solver;
    clear_chain_list(&fabrik->base.fabrik.chain_list);
    ordered_vector_clear_free(&fabrik->base.fabrik.chain_list);
    FREE(solver);
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_rebuild_data(struct solver_t* solver)
{
    return rebuild_chain_list((struct fabrik_t*)solver);
}

/* ------------------------------------------------------------------------- */
int
solver_FABRIK_solve(struct solver_t* solver)
{
    return -1;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
int
mark_involved_nodes(struct fabrik_t* solver, struct bstv_t* involved_nodes)
{
    /*
     * Traverse the chain of parents starting at each effector node and ending
     * at the root node of the tree and mark every node on the way.
     */
    struct node_t* root = solver->base.solver.private_.tree;
    struct ordered_vector_t* effector_nodes_list = &solver->base.solver.private_.effector_nodes_list;
    ORDERED_VECTOR_FOR_EACH(effector_nodes_list, struct node_t*, effector_node)
        struct node_t* node;
        for(node = *effector_node; node != root; node = node->parent)
        {
            /* NOTE Insert 1 instead of NULL so we can use bstv_erase() */
            if(bstv_insert(involved_nodes, node->guid, (void*)1) < 0)
            {
                ik_log_message("Ran out of memory while marking involved nodes");
                return -1;
            }
        }
    ORDERED_VECTOR_END_EACH

    /* mark root node as well */
    /* NOTE Insert 1 instead of NULL so we can use bstv_erase() */
    if(bstv_insert(involved_nodes, root->guid, (void*)1) < 0)
    {
        ik_log_message("Ran out of memory while marking involved nodes");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
void
clear_chain_list(struct ordered_vector_t* chain_list)
{
    uint32_t chain_size = ordered_vector_count(chain_list);

    ORDERED_VECTOR_FOR_EACH(chain_list, struct chain_t, chain)
        FREE(chain->end_position);
        ordered_vector_clear_free(&chain->nodes);
    ORDERED_VECTOR_END_EACH

    /*
     * Every chain frees its own end position, so there remains a single
     * un-freed base position in the root node.
     */
    if(chain_size > 0)
    {
        struct chain_t* root_chain = ordered_vector_get_element(chain_list, chain_size - 1);
        FREE(root_chain->base_position);
    }

    ordered_vector_clear(chain_list);
}

/* ------------------------------------------------------------------------- */
struct chain_t*
recursively_build_chain_list(struct ordered_vector_t* chain_list,
                             struct bstv_t* involved_nodes,
                             struct node_t* chain_beginning,
                             struct node_t* current_node,
                             struct chain_t* previous_chain)
{
    struct chain_t* chain;
    struct node_t* next_chain_beginning = chain_beginning;
    struct node_t* previous_chain_beginning = NULL;
    int marked_child_count = 0;

    /* We aren't interested in nodes that aren't marked */
    if(bstv_erase(involved_nodes, current_node->guid) == NULL)
        return 0;

    /*
     * If the current node has two or more marked children, it means that the
     * current node is a sub-base node and the chain must be split at this
     * point.
     */
    BSTV_FOR_EACH(&current_node->children, struct node_t, child_guid, child)
        if(bstv_hash_exists(involved_nodes, child_guid) == 0)
            if(++marked_child_count == 2)
            {
                next_chain_beginning = current_node;
                break;
            }
    BSTV_END_EACH

    /*
     * Recurse into children of the current node.
     */
    BSTV_FOR_EACH(&current_node->children, struct node_t, child_guid, child)
        previous_chain = recursively_build_chain_list(
                chain_list,
                involved_nodes,
                next_chain_beginning,
                child,
                previous_chain);
    BSTV_END_EACH

    /*
     * If the current node is not a sub-base node and is also not a leaf node,
     * that is, it has exactly one marked child, then there is no need to
     * create a new chain.
     */
    if(marked_child_count == 1)
        return previous_chain;

    if(previous_chain != NULL)
    {
        uint32_t node_count = ordered_vector_count(&previous_chain->nodes);
        previous_chain_beginning = ordered_vector_get_element(&previous_chain->nodes, node_count - 1);
    }

    chain = ordered_vector_push_emplace(chain_list);
    if(chain == NULL)
        return NULL;

    if(previous_chain_beginning == current_node)
        chain->end_position = previous_chain->base_position;
    else
        chain->end_position = MALLOC(sizeof(struct vector3_t));

    if(previous_chain_beginning == chain_beginning)
        chain->base_position = previous_chain->base_position;
    else
        chain->base_position = MALLOC(sizeof(struct vector3_t));

    ordered_vector_construct(&chain->nodes, sizeof(struct node_t*));
    for(; current_node != chain_beginning; current_node = current_node->parent)
        ordered_vector_push(&chain->nodes, current_node);
    ordered_vector_push(&chain->nodes, chain_beginning);

    return chain;
}

/* ------------------------------------------------------------------------- */
static int
rebuild_chain_list(struct fabrik_t* solver)
{
    struct bstv_t involved_nodes;
    struct ordered_vector_t* chain_list = &solver->base.fabrik.chain_list;
    struct node_t* root = solver->base.solver.private_.tree;

    bstv_construct(&involved_nodes);
    if(mark_involved_nodes(solver, &involved_nodes) < 0)
        return -1;

    clear_chain_list(chain_list);
    if(recursively_build_chain_list(chain_list, &involved_nodes, root, root, NULL) == NULL)
        return -1;

    bstv_clear_free(&involved_nodes);

    return 0;
}
