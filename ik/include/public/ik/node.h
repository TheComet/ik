#ifndef IK_NODE_H
#define IK_NODE_H

#include "ik/config.h"
#include "ik/btree.h"

C_BEGIN

struct ik_node_data_t;
struct ik_effector_t;
struct ik_constraint_t;
struct ik_pole_t;

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

/*!
 * @brief Creates and attaches a new effector to the specified node.
 * @param[out] eff If the function succeeds, a pointer to the new effector
 * object is written to this parameter.
 * @note ik_effector_t is refcounted. The pointer returned is a borrowed
 * reference, i.e. the refcount is not incremented. If you wish to use the
 * effector elsewhere and are unsure if the node will outlive the effector, you
 * should IK_INCREF the effector.
 * @param[in] node The node to create the effector on.
 * @return Returns IK_OK if the effector is successfully created. If the node
 * already has an effector, then IK_ERR_ALREADY_HAS_ATTACHMENT is returned. If
 * there is not enough memory to allocate the effector, then IK_ERR_OUT_OF_MEMORY
 * is returned.
 */
IK_PRIVATE_API ikret_t
ik_node_create_effector(struct ik_effector_t** effector, struct ik_node_t* node);

IK_PRIVATE_API ikret_t
ik_node_attach_effector(struct ik_node_t* node, struct ik_effector_t* effector);

/*!
 * @brief Unrefs the effector and detaches it from the specified node.
 * @param[in] node Node to remove effector from. If the node doesn't have an
 * effector attached, this function does nothing.
 */
IK_PRIVATE_API void
ik_node_release_effector(struct ik_node_t* node);

IK_PRIVATE_API struct ik_effector_t*
ik_node_take_effector(struct ik_node_t* node);

IK_PRIVATE_API struct ik_effector_t*
ik_node_get_effector(const struct ik_node_t* node);

IK_PRIVATE_API ikret_t
ik_node_create_constraint(struct ik_constraint_t** effector,
                          struct ik_node_t* node);

/*!
 * @brief The constraint is attached to the specified node.
 *
 * @note Constraints are a bit strange in how they are stored. They don't apply
 * to single nodes, rather, they apply to entire segments (edges connecting
 * nodes). This is not apparent in a single chain of nodes, but becomes apparent
 * if you consider a tree structure.
 *
 *    A   C
 *     \ /
 *      B
 *      |
 *      D
 *
 * If you wanted to constrain the rotation of D, then you would add a
 * constraint to node B. If you wanted to constraint the rotation of the
 * segment B-A then you would add a constraint to node A.
 *
 * @param[in] constraint The constraint object. The node gains ownership of
 * the constraint and is responsible for its deallocation. If the node already
 * owns a constraint, then the new constraint is ignored and an error code is
 * returned.
 * @param[in] node The child of the node you wish to constrain.
 * @return Returns IK_ALREADY_HAS_ATTACHMENT if the target node already has
 * a constraint attached. IK_OK if otherwise.
 * @note You will need to rebuild the solver's tree before solving.
 */
IK_PRIVATE_API ikret_t
ik_node_attach_constraint(struct ik_node_t* node,
                          struct ik_constraint_t* constraint);

IK_PRIVATE_API void
ik_node_release_constraint(struct ik_node_t* node);

IK_PRIVATE_API struct ik_constraint_t*
ik_node_take_constraint(struct ik_node_t* node);

IK_PRIVATE_API struct ik_constraint_t*
ik_node_get_constraint(const struct ik_node_t* node);

IK_PRIVATE_API ikret_t
ik_node_create_pole(struct ik_pole_t** effector, struct ik_node_t* node);

IK_PRIVATE_API ikret_t
ik_node_attach_pole(struct ik_node_t* node, struct ik_pole_t* pole);

IK_PRIVATE_API void
ik_node_release_pole(struct ik_node_t* node);

IK_PRIVATE_API struct ik_pole_t*
ik_node_take_pole(struct ik_node_t* node);

IK_PRIVATE_API struct ik_pole_t*
ik_node_get_pole(const struct ik_node_t* node);

IK_PRIVATE_API void
ik_node_set_position(struct ik_node_t* node, const ikreal_t position[3]);
IK_PRIVATE_API const ikreal_t*
ik_node_get_position(const struct ik_node_t* node);
IK_PRIVATE_API void
ik_node_set_rotation(struct ik_node_t* node, const ikreal_t rotation[4]);
IK_PRIVATE_API const ikreal_t*
ik_node_get_rotation(const struct ik_node_t* node);
IK_PRIVATE_API void
ik_node_set_rotation_weight(struct ik_node_t* node, ikreal_t rotation_weight);
IK_PRIVATE_API ikreal_t
ik_node_get_rotation_weight(const struct ik_node_t* node);
IK_PRIVATE_API void
ik_node_set_mass(struct ik_node_t* node, ikreal_t mass);
IK_PRIVATE_API ikreal_t
ik_node_get_mass(const struct ik_node_t* node);

IK_PRIVATE_API ikreal_t
ik_node_get_distance_to_parent(const struct ik_node_t* node);
IK_PRIVATE_API const void*
ik_node_get_user_data(const struct ik_node_t* node);
IK_PRIVATE_API uintptr_t
ik_node_get_uid(const struct ik_node_t* node);

#define NODE_FOR_EACH(node, key, value) \
    BTREE_FOR_EACH(&(node)->children, struct ik_node_t, key, value)

#define NODE_END_EACH BTREE_END_EACH

C_END

#endif /* IK_NODE_H */
