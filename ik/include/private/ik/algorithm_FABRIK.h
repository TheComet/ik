#ifndef IK_ALGORITHM_FABRIK_H
#define IK_ALGORITHM_FABRIK_H

#include "ik/config.h"
#include "ik/algorithm.h"

C_BEGIN

#if defined(IK_BUILDING)

IK_PRIVATE_API uintptr_t
ik_algorithm_FABRIK_type_size(void);

IK_PRIVATE_API ikret_t
ik_algorithm_FABRIK_construct(struct ik_algorithm_t* algorithm);

IK_PRIVATE_API void
ik_algorithm_FABRIK_destruct(struct ik_algorithm_t* algorithm);

IK_PRIVATE_API ikret_t
ik_algorithm_FABRIK_rebuild(struct ik_algorithm_t* algorithm);

IK_PRIVATE_API ikret_t
ik_algorithm_FABRIK_solve(struct ik_algorithm_t* algorithm);

#endif /* IK_BUILDING */

C_END

#endif /* IK_ALGORITHM_FABRIK_H */
