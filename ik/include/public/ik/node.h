#ifndef IK_NODE_H
#define IK_NODE_H

#include "ik/config.h"
#include "ik/btree.h"

C_BEGIN

struct ik_node_data_t;

/*!
 * @brief Base structure used to build the tree structure to be solved.
 */
struct ik_node_t
{
    struct ik_node_data_t* node_data;
    struct ik_node_t* parent;
    struct btree_t children;  /* holds ik_node_t* objects */
};

/*!
 * @brief Creates a new node and returns it. Each node requires a unique
 * identifier.
 * @param[out] node If successful, the new node is written to this parameter.
 * @param[in] cpp_object_or_uid The value set here is the value that will be
 * passed to the ik_node_read_callback() and ik_node_write_callback() functions.
 * If you are using this library with an existing scene graph, then you can
 * specify the pointer to the corresponding C++ node for this parameter.
 * It can also be a random integer value. As long as it is globally unique.
 * You may use ik_guid() to generate a random integer if you don't care.
 */
IK_PRIVATE_API ikret_t
ik_node_create(struct ik_node_t** node, const void* user_data);

/*!
 * @brief Constructs an already allocated node.
 */
IK_PRIVATE_API ikret_t
ik_node_construct(struct ik_node_t* node, const void* user_data);

/*!
 * @brief Destructs a node, destroying all children in the process, but does
 * not deallocate the node object itself.
 */
IK_PRIVATE_API void
ik_node_destruct(struct ik_node_t* node);

/*!
 * @brief Destructs and frees the specified node. All children will be
 * transferred to the parent node.
 * @note You will need to rebuild the solver's tree before solving.
 */
IK_PRIVATE_API void
ik_node_destroy(struct ik_node_t* node);

/*!
 * @brief Destructs a node, destroying all children in the process, but does
 * not deallocate the node object itself.
 */
IK_PRIVATE_API void
ik_node_destruct_recursive(struct ik_node_t* node);

/*!
 * @brief Destructs and frees the specified node. All children will be
 * transferred to the parent node.
 * @note You will need to rebuild the solver's tree before solving.
 */
IK_PRIVATE_API void
ik_node_destroy_recursive(struct ik_node_t* node);

/*!
 * @brief Creates a new node, attaches it as a child to the specified node,
 * and returns it. Each node requires a unique identifier, which can be used
 * later to search for nodes in the tree or to store user data for later access.
 */
IK_PRIVATE_API ikret_t
ik_node_create_child(struct ik_node_t** child,
                     struct ik_node_t* parent,
                     const void* user_data);

/*!
 * @brief Attaches a node as a child to another node. The parent node gains
 * ownership of the child node and is responsible for deallocating it.
 * @note You will need to rebuild the solver's tree before solving.
 */
IK_PRIVATE_API ikret_t
ik_node_link(struct ik_node_t* parent,
             struct ik_node_t* child);

/*!
 * @brief Unlinks a node from the tree, without destroying anything. All
 * children of the unlinked node remain in tact and will no longer be
 * affiliated with the original tree.
 * @note You will need to rebuild the solver's tree before solving.
 */
IK_PRIVATE_API void
ik_node_unlink(struct ik_node_t* node);

IK_PRIVATE_API vector_size_t
ik_node_child_count(const struct ik_node_t* node);

/*!
 * @brief Searches recursively for a node in a tree with the specified global
 * identifier.
 * @return Returns NULL if the node was not found, otherwise the node is
 * returned.
 */
IK_PRIVATE_API ikret_t
ik_node_find_child(struct ik_node_t** child,
                   const struct ik_node_t* node,
                   const void* user_data);

IK_PRIVATE_API void
ik_node_set_position(struct ik_node_t* node, const ikreal_t position[3]);
IK_PRIVATE_API void
ik_node_set_rotation(struct ik_node_t* node, const ikreal_t rotation[4]);
IK_PRIVATE_API void
ik_node_set_rotation_weight(struct ik_node_t* node, ikreal_t rotation_weight);
IK_PRIVATE_API void
ik_node_set_mass(struct ik_node_t* node, ikreal_t mass);

IK_PRIVATE_API const ikreal_t*
ik_node_get_position(const struct ik_node_t* node);
IK_PRIVATE_API const ikreal_t*
ik_node_get_rotation(const struct ik_node_t* node);
IK_PRIVATE_API ikreal_t
ik_node_get_rotation_weight(const struct ik_node_t* node);
IK_PRIVATE_API ikreal_t
ik_node_get_mass(const struct ik_node_t* node);

IK_PRIVATE_API ikreal_t
ik_node_get_distance_to_parent(const struct ik_node_t* node);
IK_PRIVATE_API const void*
ik_node_get_user_data(const struct ik_node_t* node);
IK_PRIVATE_API uintptr_t
ik_node_get_uid(const struct ik_node_t* node);
IK_PRIVATE_API const struct ik_effector_t*
ik_node_get_effector(const struct ik_node_t* node);
IK_PRIVATE_API const struct ik_constraint_t*
ik_node_get_constraint(const struct ik_node_t* node);
IK_PRIVATE_API const struct ik_pole_t*
ik_node_get_pole(const struct ik_node_t* node);

#define NODE_FOR_EACH(node, key, value) \
    BTREE_FOR_EACH(&(node)->children, struct ik_node_t, key, value)

#define NODE_END_EACH BTREE_END_EACH

C_END

#endif /* IK_NODE_H */
