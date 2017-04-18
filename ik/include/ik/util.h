#ifndef IK_UTIL_H
#define IK_UTIL_H

#include "ik/config.h"

C_HEADER_BEGIN

typedef struct ik_chain_t ik_chain_t;

IK_PUBLIC_API void
ik_calculate_rotation_weight_decays(ik_chain_t* root_chain);

C_HEADER_END

#endif /* IK_UTIL_H */
