#ifndef IK_NODE_H
#define IK_NODE_H

#include "ik/config.h"
#include "ik/bstv.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_BEGIN

struct ik_effector_t;
struct ik_constraint_t;
struct ik_pole_t;

/*!
 * @brief Base structure used to build the tree structure to be solved.
 */
struct ik_node_t
{
    /*!
     * @brief Allows the user of this library to store custom data per node
     * @note Can be set and retrieved directly without issue.
     *
     * This is especially useful in c++ applications which need to store the
     * "this" pointer to their own scene graph nodes. The user data can be
     * accessed in callback functions to make object calls again.
     *
     * For instance:
     * ```cpp
     * // A node in your scene graph
     * MyNode* node = GetMyNode();
     *
     * ik_solver_t* solver = ik_solver_create(SOLVER_FABRIK);
     * ik_node_t* ikNode = ik_node_create(node->GetID());
     * ikNode->user_data = node; // Store pointer to your own node object
     *
     * // ---- elsewhere ------
     * static void ApplyResultsCallback(ik_node_t* ikNode)
     * {
     *     Node* node = (Node*)ikNode->user_data; // Extract your own node object again
     *     node->SetPosition(ikNode->solved_position);
     * }
     * ```
     */
    const void* user_data;

    /*!
     * @brief If this node is an end effector, this will point to the end
     * effector's properties.
     * @note May be NULL.
     * @note This pointer should not be changed directly. You can however set
     * the target position/rotation of the effector by writing to
     * node->effector->target_position or node->effector->target_rotation.
     */
    struct ik_effector_t*   effector;
    struct ik_constraint_t* constraint;
    struct ik_pole_t*       pole;

    struct ik_node_t* parent;
    struct bstv_t children;

    union
    {
        struct
        {
            /*
             * WARNING: HAS to be in this order -- there's some hacking going on
             * in transform.c which relies on the order of ikreal_t's in transform[7].
             */
            struct ik_quat_t rotation;
            struct ik_vec3_t position;
        };
        ikreal_t transform[7];
    };

    ikreal_t rotation_weight;
    ikreal_t dist_to_parent;
    ikreal_t mass;
};

#if defined(IK_BUILDING)

IK_PRIVATE_API const void*
ik_guid(void);

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
ik_node_create(struct ik_node_t** node,
               const void* cpp_object_or_uid);

/*!
 * @brief Constructs an already allocated node.
 */
IK_PRIVATE_API ikret_t
ik_node_construct(struct ik_node_t* node,
                  const void* cpp_object_or_uid);

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
 * @brief Creates a new node, attaches it as a child to the specified node,
 * and returns it. Each node requires a unique identifier, which can be used
 * later to search for nodes in the tree or to store user data for later access.
 */
IK_PRIVATE_API ikret_t
ik_node_create_child(struct ik_node_t** child,
                     struct ik_node_t* parent,
                     const void* cpp_object_or_uid);

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
IK_PRIVATE_API struct ik_node_t*
ik_node_find_child(const struct ik_node_t* node,
                   const void* cpp_object_or_uid);

/*!
 * @brief Dumps all nodes recursively to DOT format. You can use graphviz (
 * or other compatible tools) to generate a graphic of the tree.
 */
IK_PRIVATE_API void
ik_node_dump_to_dot(const struct ik_node_t* node,
                    const char* file_name);

#endif /* IK_BUILDING */

#define NODE_FOR_EACH(node, key, value) \
    BSTV_FOR_EACH(&(node)->children, struct ik_node_t, key, value)

#define NODE_END_EACH BSTV_END_EACH

C_END

#endif /* IK_NODE_H */
