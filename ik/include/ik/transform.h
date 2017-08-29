#ifndef IK_TRANSFORM_H
#define IK_TRANSFORM_H

#include "ik/config.h"

C_HEADER_BEGIN

typedef enum ik_transform_flags_e
{
    TRANSFORM_ACTIVE = 0x01,
    TRANSFORM_ORIGINAL = 0x02,
    TRANSFORM_ROTATIONS_ONLY = 0x04
} ik_node_transform_flags_e;

IK_PUBLIC_API void
ik_tree_global_to_local(ik_node_t* node, uint8_t flags);

IK_PUBLIC_API void
ik_tree_local_to_global(ik_node_t* node, uint8_t flags);

IK_PUBLIC_API void
ik_chains_local_to_global(const vector_t* chains, uint8_t flags);

IK_PUBLIC_API void
ik_chains_global_to_local(const vector_t* chains, uint8_t flags);

IK_PUBLIC_API void
ik_chain_local_to_global(chain_t* chain, uint8_t flags);

IK_PUBLIC_API void
ik_chain_global_to_local(chain_t* chain, uint8_t flags);

C_HEADER_END

#endif /* IK_TRANSFORM_H */
