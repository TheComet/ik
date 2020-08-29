#pragma once

#include "ik/config.h"
#include "ik/attachment.h"

/*
 * Every available algorithm is registered using a unique name stored as a
 * string. Defining them like this is just for convenience.
 */
#define IK_ONE_BONE    "one bone"
#define IK_TWO_BONE    "two bone"
#define IK_FABRIK      "fabrik"
#define IK_MSS         "mss"

/*
 *
 */
#define IK_ALGORITHM_LIST \
    X(ONE_BONE)           \
    X(TWO_BONE)           \
    X(FABRIK)             \
    X(MSS)

#define IK_ALGORITHM_FEATURES_LIST \
    X(CONSTRAINTS,      constraints,      0x0001) \
    X(POLES,            poles,            0x0002) \
    X(TARGET_ROTATIONS, target_rotations, 0x0004) \
    X(INTEGRATE_RKF45,  integrate_rkf45,  0x0008)

C_BEGIN

enum ik_algorithm_feature
{
#define X(upper, lower, value) IK_ALGORITHM_##upper = value,
    IK_ALGORITHM_FEATURES_LIST
#undef X

    IK_ALGORITHM_FEATURE_COUNT
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
