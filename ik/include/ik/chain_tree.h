/*!
 * @file chain.h
 * @brief Manages synthesising the user specified tree of nodes (ik_node_t)
 * into a structure more optimal for solvers.
 *
 *
 */
#ifndef IK_CHAIN_H
#define IK_CHAIN_H

#include "ik/config.h"
#include "ik/ordered_vector.h"

C_HEADER_BEGIN

struct chain_t
{
    /*
     * List of ik_node_t* references that belong to this chain.
     * NOTE: The first node in this list is the effector (i.e. the *end* of the
     * chain). The nodes are in reverse.
     */
    ordered_vector_t nodes;
    /* list of chain_t objects */
    ordered_vector_t children;
};

struct chain_island_t
{
    chain_t       base_chain;
};

struct chain_tree_t
{
    ordered_vector_t islands; /* list of chain_island_t objects */
};

IK_PRIVATE_API void
chain_tree_construct(chain_tree_t* chain_trees);

IK_PRIVATE_API void
chain_tree_destruct(chain_tree_t* chain_trees);

IK_PRIVATE_API void
chain_island_construct(chain_island_t* chain_island);

IK_PRIVATE_API void
chain_island_destruct(chain_island_t* chain_island);

IK_PRIVATE_API chain_t*
chain_create(void);

IK_PRIVATE_API void
chain_destroy(chain_t* chain);

/*!
 * @brief Initialises an allocated chain object.
 */
IK_PRIVATE_API void
chain_construct(chain_t* chain);

/*!
 * @brief Destroys and frees all members, but does not deallocate the chain
 * object itself.
 */
IK_PRIVATE_API void
chain_destruct(chain_t* chain);

/*!
 * @brief Clears all children and nodes.
 */
IK_PRIVATE_API void
chain_clear_free(chain_t* chain);

/*!
 * @brief Breaks down the relevant nodes of the scene graph into a tree of
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
 * the chain tree.
 */
IK_PRIVATE_API int
rebuild_chain_tree(ik_solver_t* solver);

/*!
 * Computes the distances between the nodes and stores them in
 * node->segment_length. The positions used for this computation are those of
 * the active pose (node->position). For this reason, make sure that you've
 * correctly initialised the active pose before calling this function.
 *
 * The segment lengths are typically computed once during initialisation and
 * then never again. Of course, there are exceptions, such as when your tree
 * has translational motions. In this case, you will have to recalculate the
 * segment lengths every time node positions change.
 */
IK_PRIVATE_API void
calculate_segment_lengths(chain_tree_t* chain_tree);

/*!
 * @brief Counts all of the chains in the tree.
 */
IK_PRIVATE_API int
count_chains(chain_tree_t* chain_tree);

IK_PRIVATE_API void
calculate_global_rotations(chain_t* chain);

#ifdef IK_DOT_OUTPUT
/*!
 * @brief Dumps the chain tree to DOT format.
 * @param[in] base The base node of the user created tree. This is a parameter
 * because the base chain does not necessarily hold the base node of the tree
 * because the base node doesn't have to be part of the IK problem.
 * @note Doesn't necessarily have to be the base node, it will dump the tree
 * beginning at this node.
 * @param[in] chain Usually the base chain. Doesn't necessarily have to be the
 * base, in which case it will dump beginning at this chain.
 * @param[in] file_name The name of the file to dump to.
 */
IK_PRIVATE_API void
dump_to_dot(ik_node_t* base, chain_tree_t* chain_tree, const char* file_name);
#endif /* IK_DOT_OUTPUT */

C_HEADER_END

#endif /* IK_CHAIN_H */
