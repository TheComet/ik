#include "ik/ik.h"
#include "ik/init.h"
#include "ik/impl/build_info.h"
#include "ik/impl/callback.h"
#include "ik/impl/constraint_base.h"
#include "ik/impl/effector_base.h"
#include "ik/impl/log.h"
#include "ik/impl/mat3x3.h"
#include "ik/impl/node_base.h"
#include "ik/impl/node_FABRIK.h"
#include "ik/impl/node_base.h"
#include "ik/impl/pole_base.h"
#include "ik/impl/quat.h"
#include "ik/impl/solver.h"
#include "ik/impl/solver_base.h"
#include "ik/impl/solver_ONE_BONE.h"
#include "ik/impl/solver_TWO_BONE.h"
#include "ik/impl/solver_FABRIK.h"
#include "ik/impl/solver_MSS.h"
#include "ik/impl/tests.h"
#include "ik/impl/transform.h"
#include "ik/impl/vec3.h"

/* ------------------------------------------------------------------------- */
const struct ik_interface_t IKAPI = {
    ik_init,
    ik_deinit,
    { IK_CALLBACK_IMPL },
    { IK_BUILD_INFO_IMPL },
    { IK_LOG_IMPL },
    { IK_MAT3X3_IMPL },
    { IK_QUAT_IMPL },
    { IK_SOLVER_IMPL },
    { IK_TESTS_IMPL },
    { IK_TRANSFORM_IMPL },
    { IK_VEC3_IMPL },
    {
        { IK_CONSTRAINT_BASE_IMPL},
        { IK_EFFECTOR_BASE_IMPL},
        { IK_NODE_BASE_IMPL },
        { IK_POLE_BASE_IMPL },
        { IK_SOLVER_BASE_IMPL },
#define X(algorithm) { IK_CONSTRAINT_##algorithm##_IMPL },
        IK_ALGORITHMS
#undef X
#define X(algorithm) { IK_EFFECTOR_##algorithm##_IMPL },
        IK_ALGORITHMS
#undef X
#define X(algorithm) { IK_NODE_##algorithm##_IMPL },
        IK_ALGORITHMS
#undef X
#define X(algorithm) { IK_POLE_##algorithm##_IMPL },
        IK_ALGORITHMS
#undef X
#define X(algorithm) { IK_SOLVER_##algorithm##_IMPL },
        IK_ALGORITHMS
#undef X
    }
};
