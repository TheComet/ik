#ifndef IK_ALGORITHM_FABRIK_H
#define IK_ALGORITHM_FABRIK_H

#include "ik/config.h"
#include "ik/algorithm.h"

struct ik_algorithm_FABRIK_t
{
    IK_ALGORITHM_HEAD

    /* Used to push/pop transformations as the trees are iterated. This is
     * allocated in prepare() if alloca() is not supported, or if the stack
     * is larger than IK_MAX_STACK_ALLOC. */
    uint8_t* stack_buffer;
};

C_BEGIN

#if defined(IK_BUILDING)

IK_PRIVATE_API uintptr_t
ik_algorithm_FABRIK_type_size(void);

IK_PRIVATE_API ikret_t
ik_algorithm_FABRIK_construct(struct ik_algorithm_FABRIK_t* algorithm);

IK_PRIVATE_API void
ik_algorithm_FABRIK_destruct(struct ik_algorithm_FABRIK_t* algorithm);

IK_PRIVATE_API ikret_t
ik_algorithm_FABRIK_prepare(struct ik_algorithm_FABRIK_t* algorithm);

IK_PRIVATE_API ikret_t
ik_algorithm_FABRIK_solve(struct ik_algorithm_FABRIK_t* algorithm);

#endif /* IK_BUILDING */

C_END

#endif /* IK_ALGORITHM_FABRIK_H */
