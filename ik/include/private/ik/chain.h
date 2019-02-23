#ifndef IK_CHAIN_H
#define IK_CHAIN_H

#include "ik/config.h"

C_BEGIN

struct ik_chain_t
{
    struct ik_node_data_t* effector_node;
    struct ik_node_data_t* base_node;
};

C_END

#endif /* IK_CHAIN_H */
