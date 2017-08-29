#ifndef IK_UTIL_H
#define IK_UTIL_H

#include "ik/config.h"

C_HEADER_BEGIN

IK_PUBLIC_API void
ik_calculate_rotation_weight_decays(const vector_t* chains);

C_HEADER_END

#endif /* IK_UTIL_H */
