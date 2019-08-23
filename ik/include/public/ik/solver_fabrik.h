#ifndef IK_ALGORITHM_fabrik_H
#define IK_ALGORITHM_fabrik_H

#include "ik/config.h"
#include "ik/solver.h"

struct ik_solver_fabrik_t
{
    IK_SOLVER_HEAD

    /* Used to push/pop transformations as the trees are iterated. This is
     * allocated in prepare() if alloca() is not supported, or if the stack
     * is larger than IK_MAX_STACK_ALLOC. */
    union ik_vec3_t* transform_stack;
    uint32_t transform_stack_depth;

    /* Used to store the initial transform for computing rotations later */
    union ik_transform_t* initial_transforms;
};

C_BEGIN

IK_PRIVATE_API ikret_t
ik_solver_fabrik_init(struct ik_solver_fabrik_t* solver);

IK_PRIVATE_API void
ik_solver_fabrik_deinit(struct ik_solver_fabrik_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_fabrik_prepare(struct ik_solver_fabrik_t* solver);

IK_PRIVATE_API ikret_t
ik_solver_fabrik_solve(struct ik_solver_fabrik_t* solver);

C_END

#endif /* IK_ALGORITHM_fabrik_H */
