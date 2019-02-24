#ifndef EFFECTOR_H
#define EFFECTOR_H

#include "ik/config.h"
#include "ik/attachment.h"
#include "ik/vec3.h"
#include "ik/quat.h"

/*!
 * WEIGHT_NLERP Causes intermediary weight values to rotate the target around the
 * chain's base instead of linearly interpolating the target. Can be more
 * appealing if the solved tree diverges a lot from the original tree
 * during weight transitions.
 */
#define IK_EFFECTOR_FEATURES_LIST \
    X(WEIGHT_NLERP, 0x01) \
    X(KEEP_ORIENTATION, 0x02)

C_BEGIN

struct ik_node_t;
struct ik_node_data_t;

enum ik_effector_features_e
{
#define X(arg, value) IK_EFFECTOR_##arg = value,
    IK_EFFECTOR_FEATURES_LIST
#undef X

    IK_EFFECTOR_FLAGS_COUNT
};

/*!
 * @brief Specifies how a chain of nodes should be solved. The effector can
 * be attached to any node in a tree using ik_node_attach_effector(). The
 * effector specifies the target position and rotation of that node, as well
 * as how much influence the algorithm has on the tree (weight) and how many
 * child nodes are affected (chain_length).
 */
struct ik_effector_t
{
    IK_ATTACHMENT_HEAD

    /*!
     * @brief Can be set at any point, and should be updated whenever you have
     * a new target position to solve for. Specifies the global (world)
     * position where the node it is attached to should head for.
     * @note Default value is (0, 0, 0).
     */
    union ik_vec3_t target_position;

    /*!
     * @brief Can be set at any point, and should be updated whenever you have
     * a new target rotation to solve for. Specifies the global (world)
     * rotation where the node it is attached to should head for.
     * @note Default value is the identity quaternion.
     */
    union ik_quat_t target_rotation;

    /*!
     * Used internally to hold the actual target position/rotation, which will
     * be different from target_position/target_rotation if the weight is not
     * 1.0. This value is updated right after calling solve() and before the
     * solving algorithm begins.
     */
    union ik_vec3_t actual_target;

    /*!
     * @brief Specifies how much influence the algorithm has on the chain of
     * nodes. A value of 0.0 will cause the algorithm to completely ignore the
     * chain, while a value of 1.0 will cause the algorithm to try to place the
     * target node directly at target_position/target_rotation.
     *
     * This is useful for blending the algorithm in and out. For instance, if you
     * wanted to ground the legs of an animated character, you would want the
     * algorithm to do nothing during the time when the foot is in the air
     * (weight=0.0) and be fully active when the foot is on the ground
     * (weight=1.0).
     */
    ikreal_t weight;

    ikreal_t rotation_weight;
    ikreal_t rotation_decay;

    /*!
     * @brief Specifies how many parent nodes should be affected. A value of
     * 0 means all of the parents, including the base node.
     * @note Changing the chain length requires the algorithm tree to be rebuilt
     * with ik_algorithm_rebuild_tree().
     */
    uint16_t chain_length;

    /*!
     * @brief Various behavioral settings. Check the enum effector_flags_e for
     * more information.
     */
    uint16_t features;
};

#if defined(IK_BUILDING)

/*!
 * @brief Creates a new effector object. It can be attached to any node in the
 * tree using ik_node_attach_effector().
 */
IK_PRIVATE_API IK_WARN_UNUSED ikret_t
ik_effector_create(struct ik_effector_t** effector);

/*!
 * @brief Destroys and frees an effector object. This should **NOT** be called
 * on effectors that are attached to nodes. Use ik_node_destroy_effector()
 * instead.
 */
IK_PRIVATE_API void
ik_effector_destroy(struct ik_effector_t* effector);

IK_PRIVATE_API void
ik_effector_set_target_position(struct ik_effector_t* eff, const ikreal_t pos[3]);

IK_PRIVATE_API const ikreal_t*
ik_effector_get_target_position(const struct ik_effector_t* eff);

IK_PRIVATE_API void
ik_effector_set_target_rotation(struct ik_effector_t* eff, const ikreal_t rot[4]);

IK_PRIVATE_API const ikreal_t*
ik_effector_get_target_rotation(const struct ik_effector_t* eff);

IK_PRIVATE_API void
ik_effector_set_weight(struct ik_effector_t* eff, ikreal_t weight);

IK_PRIVATE_API ikreal_t
ik_effector_get_weight(const struct ik_effector_t* eff);

IK_PRIVATE_API void
ik_effector_set_rotation_weight(struct ik_effector_t* eff, ikreal_t weight);

IK_PRIVATE_API ikreal_t
ik_effector_get_rotation_weight(const struct ik_effector_t* eff);

IK_PRIVATE_API void
ik_effector_set_rotation_weight_decay(struct ik_effector_t* eff, ikreal_t decay);

IK_PRIVATE_API ikreal_t
ik_effector_get_rotation_weight_decay(const struct ik_effector_t* eff);

IK_PRIVATE_API void
ik_effector_set_chain_length(struct ik_effector_t* eff, uint16_t length);

IK_PRIVATE_API uint16_t
ik_effector_get_chain_length(const struct ik_effector_t* eff);

IK_PRIVATE_API void
ik_effector_enable_features(struct ik_effector_t* eff, uint8_t features);

IK_PRIVATE_API void
ik_effector_disable_features(struct ik_effector_t* eff, uint8_t features);

IK_PRIVATE_API uint8_t
ik_effector_get_features(const struct ik_effector_t* eff);

IK_PRIVATE_API uint8_t
ik_effector_is_feature_enabled(const struct ik_effector_t* eff,
                               enum ik_effector_features_e feature);

#endif /* IK_BUILDING */

C_END

#endif /* EFFECTOR_H */
