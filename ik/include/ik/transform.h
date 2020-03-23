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

#if defined(IK_BUILDING)

IK_PRIVATE_API void
ik_transform_node(struct ik_node* root, uint8_t mode);

IK_PRIVATE_API void
ik_transform_chain(struct ik_chain* chain, uint8_t mode);

IK_PRIVATE_API void
ik_transform_chain_list(const struct ik_algorithm* algorithm, uint8_t mode);

#endif /* IK_BUILDING */

C_END

#endif /* IK_TRANSFORM_H */
