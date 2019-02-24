#ifndef IK_ALGORITHM_ONE_BONE_H
#define IK_ALGORITHM_ONE_BONE_H

#include "ik/config.h"
#include "ik/algorithm.h"

C_BEGIN

#if defined(IK_BUILDING)

IK_PRIVATE_API uintptr_t
ik_algorithm_ONE_BONE_type_size(void);

IK_PRIVATE_API ikret_t
ik_algorithm_ONE_BONE_construct(struct ik_algorithm_t* algorithm);

IK_PRIVATE_API void
ik_algorithm_ONE_BONE_destruct(struct ik_algorithm_t* algorithm);

IK_PRIVATE_API ikret_t
ik_algorithm_ONE_BONE_prepare(struct ik_algorithm_t* algorithm);

IK_PRIVATE_API ikret_t
ik_algorithm_ONE_BONE_solve(struct ik_algorithm_t* algorithm);

#endif /* IK_BUILDING */

C_END

#endif /* IK_ALGORITHM_ONE_BONE_H */
