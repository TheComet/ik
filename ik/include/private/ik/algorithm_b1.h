#ifndef IK_ALGORITHM_ONE_BONE_H
#define IK_ALGORITHM_ONE_BONE_H

#include "ik/config.h"
#include "ik/algorithm.h"

C_BEGIN

struct ik_algorithm_b1_t
{
    IK_ALGORITHM_HEAD
};

IK_PRIVATE_API uintptr_t
ik_algorithm_b1_type_size(void);

IK_PRIVATE_API ikret_t
ik_algorithm_b1_construct(struct ik_algorithm_b1_t* algorithm);

IK_PRIVATE_API void
ik_algorithm_b1_destruct(struct ik_algorithm_b1_t* algorithm);

IK_PRIVATE_API ikret_t
ik_algorithm_b1_prepare(struct ik_algorithm_b1_t* algorithm);

IK_PRIVATE_API ikret_t
ik_algorithm_b1_solve(struct ik_algorithm_b1_t* algorithm);

C_END

#endif /* IK_ALGORITHM_ONE_BONE_H */
