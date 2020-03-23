#ifndef IK_ALGORITHM_H
#define IK_ALGORITHM_H

#include "ik/config.h"
#include "ik/attachment.h"

/*!
 * @brief Only the solvers listed here are actually enabled.
 *
 * This list is used at multiple locations in the library, specifically in
 * ik_solver_base_create() to fill in the switch/case with information on how
 * to create each solver, but there are also certain expectations on how
 * functions are to be named for each solver.
 */
#define IK_SOLVER_ALGORITHM_LIST \
    X(DUMMY1,   dummy1) \
    X(DUMMY2,   dummy2) \
    X(ONE_BONE, b1) \
    X(TWO_BONE, b2) \
    X(FABRIK,   fabrik)
    /*X(MSS, mss)*/

#define IK_SOLVER_FEATURES_LIST \
    X(CONSTRAINTS,      0x0001) \
    X(TARGET_ROTATIONS, 0x0002) \
    X(JOINT_ROTATIONS,  0x0004) \
    X(ALL,              0xFFFF)

C_BEGIN

enum ik_algorithm_type
{
#define X(upper, lower) IK_SOLVER_##upper,
    IK_SOLVER_ALGORITHM_LIST
#undef X

    IK_SOLVER_ALGORITHM_COUNT
};

enum ik_algorithm_feature
{
#define X(name, value) IK_SOLVER_##name,
    IK_SOLVER_FEATURES_LIST
#undef X

    IK_SOLVER_FEATURE_COUNT
};

struct ik_algorithm
{
    IK_ATTACHMENT_HEAD

    ikreal tolerance;
    uint16_t features;
    uint16_t max_iterations;
    enum ik_algorithm_type algorithm;
};

struct ik_algorithm_interface
{
    struct ik_algorithm* (*create)(enum ik_algorithm_type algorithm);
    void (*destroy)(struct ik_algorithm* algorithm);
};

IK_PRIVATE_API struct ik_algorithm*
ik_algorithm_create(void);

IK_PRIVATE_API void
ik_algorithm_destroy(struct ik_algorithm* algorithm);

C_END

#endif /* IK_ALGORITHM_H */
