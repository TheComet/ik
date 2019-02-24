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

struct ik_algorithm_t;
struct vector_t;
struct ik_node_t;
struct chain_t;


enum ik_transform_mode_e
{
#define X(arg, value) IK_TRANSFORM_##arg = value,
    IK_TRANSFORM_MODE_LIST
#undef X

    IK_TRANSFORM_MODE_COUNT
};

union ik_transform_t
{
    struct
    {
        /*
         * WARNING: HAS to be in this order -- there's some hacking going on
         * in transform.c which relies on the order of ikreal_t's in transform[7].
         */
        union ik_quat_t rotation;
        union ik_vec3_t position;
    } t;
    ikreal_t f[7];
};

#if defined(IK_BUILDING)

IK_PRIVATE_API void
ik_transform_node(struct ik_node_t* root, uint8_t mode);

IK_PRIVATE_API void
ik_transform_chain(struct chain_t* chain, uint8_t mode);

IK_PRIVATE_API void
ik_transform_chain_list(const struct ik_algorithm_t* algorithm, uint8_t mode);

#endif /* IK_BUILDING */

C_END

#endif /* IK_TRANSFORM_H */
