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
struct ik_node_interface_t;

#define IK_NODE_HEAD                                                          \
    const struct ik_node_interface_t* v;                                      \
    struct ik_node_t* parent;                                                 \
    struct bstv_t children;                                                   \
    uint32_t guid;                                                            \
                                                                              \
    union                                                                     \
    {                                                                         \
        struct                                                                \
        {                                                                     \
            /*                                                                \
             * WARNING: HAS to be in this order -- there's some hacking going on \
             * in transform.c which relies on the order of ikreal_t's in transform[7]. \
             */                                                               \
            struct ik_quat_t rotation;                                        \
            struct ik_vec3_t position;                                        \
        };                                                                    \
        ikreal_t transform[7];                                                \
    };                                                                        \
                                                                              \
    /*!                                                                       \
     * @brief Allows the user of this library to store custom data per node   \
     * @note Can be set and retrieved directly without issue.                 \
     *                                                                        \
     * This is especially useful in c++ applications which need to store the  \
     * "this" pointer to their own scene graph nodes. The user data can be    \
     * accessed in callback functions to make object calls again.             \
     *                                                                        \
     * For instance:                                                          \
     * ```cpp                                                                 \
     * // A node in your scene graph                                          \
     * MyNode* node = GetMyNode();                                            \
     *                                                                        \
     * ik_solver_t* solver = ik_solver_create(SOLVER_FABRIK);                 \
     * ik_node_t* ikNode = ik_node_create(node->GetID());                     \
     * ikNode->user_data = node; // Store pointer to your own node object     \
     *                                                                        \
     * // ---- elsewhere ------                                               \
     * static void ApplyResultsCallback(ik_node_t* ikNode)                    \
     * {                                                                      \
     *     Node* node = (Node*)ikNode->user_data; // Extract your own node object again \
     *     node->SetPosition(ikNode->solved_position);                        \
     * }                                                                      \
     * ```                                                                    \
     */                                                                       \
    void* user_data;                                                          \
                                                                              \
    /*!                                                                       \
     * @brief If this node is an end effector, this will point to the end     \
     * effector's properties.                                                 \
     * @note May be NULL.                                                     \
     * @note This pointer should not be changed directly. You can however set \
     * the target position/rotation of the effector by writing to             \
     * node->effector->target_position or node->effector->target_rotation.    \
     */                                                                       \
    struct ik_effector_t*   effector;                                         \
    struct ik_constraint_t* constraint;                                       \
    struct ik_pole_t*       pole;                                             \
                                                                              \
    ikreal_t rotation_weight;                                                 \
    ikreal_t dist_to_parent;

/*!
 * @brief Base structure used to build the tree structure to be solved.
 */
struct ik_node_t
{
    IK_NODE_HEAD
};

IK_INTERFACE(node_interface)
{
    /*!
     * @brief Creates a new node and returns it. Each node requires a tree-unique
     * ID, which can be used later to search for nodes in the tree.
     */
    struct ik_node_t*
    (*create)(uint32_t guid);

    /*!
     * @brief Constructs an already allocated node.
     */
    ikret_t
    (*construct)(struct ik_node_t* node, uint32_t guid);

    /*!
     * @brief Destructs a node, destroying all children in the process, but does
     * not deallocate the node object itself.
     */
    void
    (*destruct)(struct ik_node_t* node);

    /*!
     * @brief Destructs and frees the node, destroying all children in the process.
     * If the node was part of a tree, then it will be removed from its parents.
     * @note You will need to rebuild the solver's tree before solving.
     */
    void
    (*destroy)(struct ik_node_t* node);

    /*!
     * @brief Creates a new node, attaches it as a child to the specified node,
     * and returns it. Each node requires a tree-unique ID, which can be used
     * later to search for nodes in the tree.
     */
    struct ik_node_t*
    (*create_child)(struct ik_node_t* node, uint32_t child_guid);

    /*!
     * @brief Attaches a node as a child to another node. The parent node gains
     * ownership of the child node and is responsible for deallocating it.
     * @note You will need to rebuild the solver's tree before solving.
     */
    ikret_t
    (*add_child)(struct ik_node_t* node, struct ik_node_t* child);

    /*!
     * @brief Unlinks a node from the tree, without destroying anything. All
     * children of the unlinked node remain in tact and will no longer be
     * affiliated with the original tree.
     * @note You will need to rebuild the solver's tree before solving.
     */
    void
    (*unlink)(struct ik_node_t* node);

    /*!
     * @brief Searches recursively for a node in a tree with the specified global
     * identifier.
     * @return Returns NULL if the node was not found, otherwise the node is
     * returned.
     */
    struct ik_node_t*
    (*find_child)(const struct ik_node_t* node, uint32_t guid);

    struct ik_node_t*
    (*duplicate)(const struct ik_node_t* node, int copy_attachments);

    /*!
     * @brief Dumps all nodes recursively to DOT format. You can use graphviz (
     * or other compatible tools) to generate a graphic of the tree.
     */
    void
    (*dump_to_dot)(struct ik_node_t* node, const char* file_name);
};

#define NODE_FOR_EACH(node, key, value) \
    BSTV_FOR_EACH(&(node)->children, struct ik_node_t, key, value)

#define NODE_END_EACH BSTV_END_EACH

C_END

#endif /* IK_NODE_H */
