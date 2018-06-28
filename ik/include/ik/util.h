#ifndef IK_UTIL_H
#define IK_UTIL_H

#include "ik/config.h"

C_HEADER_BEGIN

/* XXX Add a function for only calculating it for a given effector? */

IK_PRIVATE_API void
ik_calculate_rotation_weight_decays(const struct vector_t* chains);

C_HEADER_END

#endif /* IK_UTIL_H */
