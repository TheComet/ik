/*!
 * @file chain_tree.h
 * @brief Manages synthesizing the user specified tree of bones (ik_bone)
 * into a structure more optimal for solvers.
 */
#pragma once

#include "ik/config.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_bone;
struct ik_subtree;

struct ik_chain
{
    /*
     * List of ik_bone* references that belong to this chain.
     * NOTE: The bones are in "reverse", i.e. the first bone in this list is
     * the effector bone.
     */
    struct cs_vector bones;
    /* list of ik_chain objects */
    struct cs_vector children;
};

IK_PRIVATE_API struct ik_chain*
chain_tree_create(void);

IK_PRIVATE_API void
chain_tree_destroy(struct ik_chain* chain);

/*!
 * @brief Initializes an allocated chain object.
 */
IK_PRIVATE_API void
chain_tree_init(struct ik_chain* chain);

/*!
 * @brief Destroys and frees all members, but does not deallocate the chain
 * object itself.
 */
IK_PRIVATE_API void
chain_tree_deinit(struct ik_chain* chain);

/*!
 * @brief Deletes all children and bones.
 */
IK_PRIVATE_API void
chain_tree_clear(struct ik_chain* chain);

IK_PRIVATE_API struct ik_chain*
chain_create_child(struct ik_chain* chain);

IK_PRIVATE_API int
chain_add_bone(struct ik_chain* chain, const struct ik_bone* bone);

IK_PRIVATE_API int
chain_tree_build(struct ik_chain* chain, const struct ik_subtree* subtree);

/*!
 * @brief Counts all of the chains in the tree.
 */
IK_PRIVATE_API int
chain_count(const struct ik_chain* chain);

/*!
 * @brief Counts all of the bones in the chain tree.
 */
IK_PRIVATE_API int
chain_count_bones(const struct ik_chain* chain);

/*!
 * @brief Helper macro for retrieving the bone by index.
 * @note Does no error checking at all (e.g. if the index is out of bounds).
 */
#define chain_get_bone(chain_var, idx) \
    (*(struct ik_bone**)vector_get_element(&(chain_var)->bones, idx))

#define chain_get_child(chain_var, idx) \
    ((struct ik_chain*)vector_get_element(&(chain_var)->children, idx))

/*!
 * @brief Helper macro for retrieving the number of bones in a chain.
 */
#define chain_bone_count(chain_var) \
    (vector_count(&(chain_var)->bones))

/*!
 * @brief Helper macro for retrieving the base bone in the chain.
 * @note Does no error checking at all.
 */
#define chain_get_base_bone(chain_var) \
    (*(struct ik_bone**)vector_get_element(&(chain_var)->bones, chain_bone_count(chain_var) - 1))

/*!
 * @brief Helper macro for retrieving the last bone in the chain.
 * @note Does no error checking at all.
 */
#define chain_get_tip_bone(chain_var) \
    (chain_get_bone(chain_var, 0))

#define chain_child_count(chain_var) \
    (vector_count(&(chain_var)->children))

#define CHAIN_FOR_EACH_CHILD(chain_var, var_name) \
    VECTOR_FOR_EACH(&(chain_var)->children, struct ik_chain, var_name) {

#define CHAIN_FOR_EACH_CHILD_R(chain_var, var_name) \
    VECTOR_FOR_EACH_R(&(chain_var)->children, struct ik_chain, var_name) {

/*!
 * Iterates over each bone from tip to base.
 */
#define CHAIN_FOR_EACH_BONE(chain_var, var_name) \
    VECTOR_FOR_EACH(&(chain_var)->bones, struct ik_bone*, chain_##var_name) \
    struct ik_bone* var_name = *(chain_##var_name); {

/*!
 * Iterates over each bone from base to tip.
 */
#define CHAIN_FOR_EACH_BONE_R(chain_var, var_name) \
    VECTOR_FOR_EACH_R(&(chain_var)->bones, struct ik_bone*, chain_##var_name) \
    struct ik_bone* var_name = *(chain_##var_name); {

#define CHAIN_FOR_EACH_BONE_PAIR(chain_var, parent_var, child_var) {          \
    cs_vec_idx idx_##parent_var;                                              \
    for (idx_##parent_var = 0; idx_##parent_var < chain_bone_count(chain_var) - 1; ++idx_##parent_var) { \
        struct ik_bone* parent_var = chain_get_bone(chain_var, idx_##parent_var + 1); \
        struct ik_bone* child_var = chain_get_bone(chain_var, idx_##parent_var + 0); {

#define CHAIN_FOR_EACH_BONE_PAIR_R(chain_var, parent_var, child_var) {        \
    cs_vec_idx idx_##parent_var;                                              \
    for (idx_##parent_var = chain_bone_count(chain_var) - 1; idx_##parent_var > 0; --idx_##parent_var) { \
        struct ik_bone* parent_var = chain_get_bone(chain_var, idx_##parent_var - 0); \
        struct ik_bone* child_var = chain_get_bone(chain_var, idx_##parent_var - 1); {

#define CHAIN_END_EACH \
    VECTOR_END_EACH }

#ifdef IK_DOT_OUTPUT
/*!
 * @brief Dumps the chain tree to DOT format.
 * @param[in] base The base bone of the user created tree. This is a parameter
 * because the base chain does not necessarily hold the base bone of the tree
 * because the base bone doesn't have to be part of the IK problem.
 * @note Doesn't necessarily have to be the base bone, it will dump the tree
 * beginning at this bone.
 * @param[in] chains A vector of base chains to dump.
 * @param[in] file_name The name of the file to dump to.
 */
IK_PRIVATE_API void
dump_to_dot(const struct ik_bone* bone, const struct cs_vector* chains, const char* file_name);
#endif /* IK_DOT_OUTPUT */

C_END
