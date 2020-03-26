/*!
 * @file chain.h
 * @brief Manages synthesising the user specified tree of nodes (ik_node)
 * into a structure more optimal for solvers.
 */
#ifndef IK_CHAIN_H
#define IK_CHAIN_H

#include "ik/config.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_node;
struct ik_subtree;

struct ik_chain
{
    /*
     * List of ik_node* references that belong to this chain.
     * NOTE: The nodes are in "reverse", i.e. the first node in this list is
     * the effector node.
     */
    struct vector_t nodes;
    /* list of ik_chain objects */
    struct vector_t children;
};

IK_PRIVATE_API struct ik_chain*
chain_create(void);

IK_PRIVATE_API void
chain_destroy(struct ik_chain* chain);

/*!
 * @brief Initializes an allocated chain object.
 */
IK_PRIVATE_API int
chain_init(struct ik_chain* chain);

/*!
 * @brief Destroys and frees all members, but does not deallocate the chain
 * object itself.
 */
IK_PRIVATE_API void
chain_deinit(struct ik_chain* chain);

/*!
 * @brief Deletes all children and nodes.
 */
IK_PRIVATE_API void
chain_clear(struct ik_chain* chain);

IK_PRIVATE_API struct ik_chain*
chain_create_child(struct ik_chain* chain);

IK_PRIVATE_API int
chain_add_node(struct ik_chain* chain, const struct ik_node* node);

IK_PRIVATE_API int
chain_tree_rebuild(struct ik_chain* chain, const struct ik_subtree* subtree);

/*!
 * @brief Counts all of the chains in the tree.
 */
IK_PRIVATE_API int
count_chains(const struct ik_chain* chains);

/*!
 * @brief Helper macro for retrieving the node by index.
 * @note Does no error checking at all (e.g. if the index is out of bounds).
 */
#define chain_get_node(chain_var, idx) \
    (*(struct ik_node**)vector_get_element(&(chain_var)->nodes, idx))

/*!
 * @brief Helper macro for retrieving the number of nodes in a chain.
 */
#define chain_length(chain_var) \
    vector_count(&(chain_var)->nodes)

/*!
 * @brief Helper macro for retrieving the base node in the chain.
 * @note Does no error checking at all.
 */
#define chain_get_base_node(chain_var) \
    (*(struct ik_node**)vector_get_element(&(chain_var)->nodes, \
                                chain_length(chain_var) - 1))

/*!
 * @brief Helper macro for retrieving the last node in the chain.
 * @note Does no error checking at all.
 */
#define chain_get_tip_node(chain_var) \
    chain_get_node(chain_var, 0)

#define chain_child_count(chain_var) \
    (vector_count(&(chain_var)->children))

#define CHAIN_FOR_EACH_CHILD(chain_var, var_name) \
    VECTOR_FOR_EACH(&(chain_var)->children, struct ik_chain, var_name) {

#define CHAIN_FOR_EACH_NODE(chain_var, var_name) \
    VECTOR_FOR_EACH(&(chain_var)->nodes, struct ik_node*, chain_##var_name) \
    struct ik_node* var_name = *(chain_##var_name); {

#define CHAIN_END_EACH VECTOR_END_EACH }

#ifdef IK_DOT_OUTPUT
/*!
 * @brief Dumps the chain tree to DOT format.
 * @param[in] base The base node of the user created tree. This is a parameter
 * because the base chain does not necessarily hold the base node of the tree
 * because the base node doesn't have to be part of the IK problem.
 * @note Doesn't necessarily have to be the base node, it will dump the tree
 * beginning at this node.
 * @param[in] chains A vector of base chains to dump.
 * @param[in] file_name The name of the file to dump to.
 */
IK_PRIVATE_API void
dump_to_dot(const struct ik_node* node, const struct vector_t* chains, const char* file_name);
#endif /* IK_DOT_OUTPUT */

C_END

#endif /* IK_CHAIN_H */
