#ifndef IK_NODE_DATA_H
#define IK_NODE_DATA_H

#include "ik/config.h"
#include "ik/refcounted.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_BEGIN

struct ik_effector_t;
struct ik_constraint_t;
struct ik_pole_t;

struct ik_node_data_t
{
    IK_REFCOUNTED(struct ik_node_data_t)

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

    ikreal_t dist_to_parent;

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
    ikreal_t mass;
};

IK_PRIVATE_API ikret_t
ik_node_data_create(struct ik_node_data_t** node_data, const void* user_data);

IK_PRIVATE_API ikret_t
ik_node_data_construct(struct ik_node_data_t* node_data, const void* user_data);

C_END

#endif /* IK_NODE_DATA_H */
