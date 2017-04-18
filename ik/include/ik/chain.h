/*!
 * @file chain.h
 * @brief Manages synthesising the user specified tree of nodes (ik_node_t)
 * into a tree of chains, optimal for solvers.
 */
#ifndef IK_CHAIN_H
#define IK_CHAIN_H

#include "ik/config.h"
#include "ik/ordered_vector.h"

C_HEADER_BEGIN

typedef struct ik_solver_t ik_solver_t;
typedef struct ik_node_t ik_node_t;

typedef struct ik_chain_t
{
    /* list of node_t* references that belong to this chain */
    ordered_vector_t nodes;
    /* list of chain_t objects */
    ordered_vector_t children;
} ik_chain_t;

ik_chain_t*
chain_create(void);

void
chain_destroy(ik_chain_t* chain);

/*!
 * @brief Initialises an allocated chain object.
 */
void
chain_construct(ik_chain_t* chain);

/*!
 * @brief Destroys and frees all members, but does not deallocate the chain
 * object itself.
 */
void
chain_destruct(ik_chain_t* chain);


/*!
 * @brief Clears all children and nodes.
 */
void
chain_clear_free(ik_chain_t* chain);

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
int
rebuild_chain_tree(ik_solver_t* solver);

void
calculate_segment_lengths(ik_chain_t* chain);

/*!
 * @brief Counts all of the chains in the tree, excluding the root chain.
 */
int
count_chains_exclude_root(ik_chain_t* chain);

#if IK_DOT_OUTPUT == ON
/*!
 * @brief Dumps the chain tree to DOT format.
 * @param[in] root The root node of the user created tree. This is a parameter
 * because the root chain does not necessarily hold the root node of the tree
 * because the root node doesn't have to be part of the IK problem.
 * @note Doesn't necessarily have to be the root node, it will dump the tree
 * beginning at this node.
 * @param[in] chain Usually the root chain. Doesn't necessarily have to be the
 * root, in which case it will dump beginning at this chain.
 * @param[in] file_name The name of the file to dump to.
 */
void
dump_to_dot(ik_node_t* root, ik_chain_t* chain, const char* file_name);
#endif /* IK_DOT_OUTPUT */

C_HEADER_END

#endif /* IK_CHAIN_H */
