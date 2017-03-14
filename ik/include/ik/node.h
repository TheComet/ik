#ifndef IK_NODE_H
#define IK_NODE_H

#include "ik/gen/config.h"
#include "ik/pstdint.h"
#include "ik/bst_vector.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_HEADER_BEGIN

struct ik_effector_t;

/*!
 * @brief Represents one node in the tree to be solved.
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
     */
    void* user_data;

    /*!
     * @brief The initial global position (in world space).
     * @note Must be set by the user to get correct results. This value can
     * be set and retrieved at any time.
     * @note The default value is (0, 0, 0).
     */
    vec3_t position;

    /*!
     * @brief The initial global rotation (in world space).
     * @note Must be set by the user to get correct results if the solver has
     * angle computations enabled.
     * @note The default value is the identity quaternion.
     */
    quat_t rotation;

    uint32_t guid;

    vec3_t solved_position;
    quat_t solved_rotation;
    ik_real segment_length;

    struct ik_node_t* parent;
    struct bstv_t children;
    struct ik_effector_t* effector;
};

IK_PUBLIC_API struct ik_node_t*
ik_node_create(uint32_t guid);

IK_PUBLIC_API void
ik_node_construct(struct ik_node_t* node, uint32_t guid);

IK_PUBLIC_API void
ik_node_destruct(struct ik_node_t* node);

IK_PUBLIC_API void
ik_node_destroy(struct ik_node_t* node);

IK_PUBLIC_API void
ik_node_add_child(struct ik_node_t* node, struct ik_node_t* child);

IK_PUBLIC_API void
ik_node_unlink(struct ik_node_t* node);

IK_PUBLIC_API struct ik_node_t*
ik_node_find_child(struct ik_node_t* node, uint32_t guid);

IK_PUBLIC_API void
ik_node_attach_effector(struct ik_node_t* node, struct ik_effector_t* effector);

IK_PUBLIC_API void
ik_node_destroy_effector(struct ik_node_t* node);

IK_PUBLIC_API void
ik_node_dump_to_dot(struct ik_node_t* node, const char* file_name);

C_HEADER_END

#endif /* IK_NODE_H */
