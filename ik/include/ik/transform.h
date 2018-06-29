#ifndef IK_TRANSFORM_H
#define IK_TRANSFORM_H

#include "ik/config.h"

C_BEGIN

struct vector_t;
struct ik_node_t;
struct chain_t;

enum ik_transform_flags_e
{
    TR_G2L            = 0x00,
    TR_L2G            = 0x01,
    TR_ROTATIONS      = 0x02,
    TR_TRANSLATIONS   = 0x04
};

IK_PRIVATE_API void
ik_transform_tree(struct ik_node_t* node, uint8_t flags);

IK_PRIVATE_API void
ik_transform_chain_list(const struct vector_t* chain_list, uint8_t flags);

IK_PRIVATE_API void
ik_transform_chain(struct chain_t* chain, uint8_t flags);

C_END

#endif /* IK_TRANSFORM_H */
