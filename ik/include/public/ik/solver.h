#ifndef IK_SOLVE_POINT_H
#define IK_SOLVE_POINT_H

#include "ik/config.h"
#include "ik/vector.h"

#define IK_SOLVE_POINT_FEATURES_LIST \
    X(CONSTRAINTS,      0x01) \
    X(TARGET_ROTATIONS, 0x02) \
    X(JOINT_ROTATIONS,  0x04) \
    X(ALL,              0xFF)

C_BEGIN

struct ik_node_t;

enum ik_solve_point_feature_e
{
#define X(arg, value) IK_ALGORITHM_##arg = value,
    IK_SOLVE_POINT_FEATURES_LIST
#undef X

    IK_SOLVE_POINT_FEATURES_COUNT
};
struct ik_solve_point_t
{
    /* Reference to algorithm to use for the assigned NTF */
    struct ik_algorithm_t* algorighm;

    /* Reference to flattened tree this solve_point is responsible for solving  */
    struct ik_ntf_t* ntf;
};

IK_PRIVATE_API IK_WARN_UNUSED ikret_t
ik_solve_point_create(struct ik_solve_point_t** solve_point);

IK_PRIVATE_API IK_WARN_UNUSED ikret_t
ik_solve_point_construct(struct ik_solve_point_t* solve_point);

IK_PRIVATE_API void
ik_solve_point_destruct(struct ik_solve_point_t* solve_point);

IK_PRIVATE_API void
ik_solve_point_destroy(struct ik_solve_point_t* solve_point);

IK_PRIVATE_API ikret_t
ik_solve_point_prepare(struct ik_solve_point_t* solve_point, struct ik_node_t* tree);

IK_PRIVATE_API ikret_t
ik_solve_point_update(struct ik_solve_point_t* solve_point);

IK_PRIVATE_API uint32_t
ik_solve_point_solve(struct ik_solve_point_t* solve_point);

C_END

#endif /* IK_SOLVE_POINT_H */
