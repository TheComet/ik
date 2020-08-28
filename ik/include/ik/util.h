#pragma once

#include "ik/config.h"

C_BEGIN

#if defined(IK_BUILDING)

/* XXX Add a function for only calculating it for a given effector? */

IK_PRIVATE_API void
ik_calculate_rotation_weight_decays(const struct vector_t* chains);

#endif /* IK_BUILDING */

C_END
