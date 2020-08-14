#ifndef IK_TRANSFORM_H
#define IK_TRANSFORM_H

#include "ik/config.h"
#include "ik/quat.h"
#include "ik/vec3.h"

#define IK_TRANSFORM_MODE_LIST \
    X(G2L, 0x00) \
    X(L2G, 0x01) \
    X(ROTATIONS, 0x02) \
    X(TRANSLATIONS, 0x04)

C_BEGIN

struct ik_algorithm;
struct ik_node;
struct ik_chain;
struct cs_vector;

enum ik_transform_mode
{
#define X(arg, value) IK_TRANSFORM_##arg = value,
    IK_TRANSFORM_MODE_LIST
#undef X

    IK_TRANSFORM_MODE_COUNT
};

union ik_transform
{
    struct
    {
        /*
         * WARNING: HAS to be in this order -- there's some hacking going on
         * in transform.c which relies on the order of ikreal_t's in transform[7].
         */
        union ik_quat rot;
        union ik_vec3 pos;
    } t;
    ikreal f[7];
};

/*!
 * Transforms a position and rotation from the node space of "tip" into the
 * node space of "base".
 * @note The nodes tip and base must be in local space for this to work.
 */
IK_PUBLIC_API void
ik_transform_pos_rot_l2g(ikreal pos[3], ikreal rot[4], const struct ik_node* tip, const struct ik_node* base);

/*!
 * Transforms a position and rotation from the node space of "base" into the
 * node space of "tip".
 * @note The nodes tip and base must be in local space for this to work.
 */
IK_PUBLIC_API void
ik_transform_pos_rot_g2l(ikreal pos[3], ikreal rot[4], const struct ik_node* tip, const struct ik_node* base);

/*!
 * Transforms a position from the node space of "tip" into the node space of
 * "base".
 * @note The nodes tip and base must be in local space for this to work.
 */
IK_PUBLIC_API void
ik_transform_pos_l2g(ikreal pos[3], const struct ik_node* tip, const struct ik_node* base);

/*!
 * Transforms a position from the node space of "base" into the node space of
 * "tip".
 * @note The nodes tip and base must be in local space for this to work.
 */
IK_PUBLIC_API void
ik_transform_pos_g2l(ikreal pos[3], const struct ik_node* tip, const struct ik_node* base);

/*!
 * Transforms all nodes ranging from "tip" to "base" into the space of "base".
 */
IK_PUBLIC_API void
ik_transform_node_section_l2g(struct ik_node* tip, const struct ik_node* base);

/*!
 * Transforms all nodes ranging from "tip" to "base" back to local space.
 */
IK_PUBLIC_API void
ik_transform_node_section_g2l(struct ik_node* tip, const struct ik_node* base);

IK_PRIVATE_API void
ik_transform_chain_to_segmental_representation(struct ik_chain* root,
                                               union ik_quat* intermediate_rotations,
                                               int num_intermediate_rotations);

IK_PRIVATE_API void
ik_transform_chain_to_nodal_representation(struct ik_chain* root,
                                           union ik_quat* intermediate_rotations,
                                           int num_intermediate_rotations);

C_END

#endif /* IK_TRANSFORM_H */
