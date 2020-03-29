#ifndef IK_ALGORITHM_H
#define IK_ALGORITHM_H

#include "ik/config.h"
#include "ik/attachment.h"

#define IK_ONE_BONE "one bone"
#define IK_TWO_BONE "two bone"
#define IK_FABRIK   "fabrik"
#define IK_MSS      "mss"

#define IK_ALGORITHM_LIST \
    X(ONE_BONE)           \
    X(TWO_BONE)           \
    /*X(FABRIK)             \
    X(MSS)*/

#define IK_ALGORITHM_FEATURES_LIST \
    X(CONSTRAINTS,      0x0001) \
    X(TARGET_ROTATIONS, 0x0002) \
    X(JOINT_ROTATIONS,  0x0004) \
    X(ALL,              0xFFFF)

C_BEGIN

enum ik_algorithm_feature
{
#define X(name, value) IK_SOLVER_##name = value,
    IK_ALGORITHM_FEATURES_LIST
#undef X

    IK_SOLVER_FEATURE_COUNT
};

struct ik_algorithm
{
    IK_ATTACHMENT_HEAD

    char type[16];

    ikreal tolerance;
    uint16_t features;
    uint16_t max_iterations;
};

IK_PUBLIC_API struct ik_algorithm*
ik_algorithm_create(const char* name);

IK_PUBLIC_API int
ik_algorithm_set_type(struct ik_algorithm* algorighm, const char* name);

C_END

#endif /* IK_ALGORITHM_H */
