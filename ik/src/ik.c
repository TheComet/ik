#include "ik/ik.h"
#include "ik/build_info_static.h"
#include "ik/constraint_base.h"
#include "ik/effector_base.h"
#include "ik/log_static.h"
#include "ik/memory.h"
#include "ik/node_base.h"
#include "ik/node_FABRIK.h"
#include "ik/quat_static.h"
#include "ik/solver_static.h"
#include "ik/solver_base.h"
#include "ik/solver_ONE_BONE.h"
#include "ik/solver_TWO_BONE.h"
#include "ik/solver_FABRIK.h"
#include "ik/solver_MSS.h"
#include "ik/tests_static.h"
#include "ik/vec3_static.h"
#include <stddef.h>
#include <stdio.h>

static int g_init_counter = 0;

/* ------------------------------------------------------------------------- */
static void
log_stdout_callback(const char* msg)
{
    puts(msg);
}

/* ------------------------------------------------------------------------- */
static const struct ik_callback_interface_t dummy_callbacks = {
    log_stdout_callback,
    NULL
};
static void
ik_implement_callbacks(const struct ik_callback_interface_t* callbacks)
{
    if (callbacks)
        IKAPI.internal.callbacks = callbacks;
    else
        IKAPI.internal.callbacks = &dummy_callbacks;
}

/* ------------------------------------------------------------------------- */
static ikret_t
ik_init(void)
{
    if (g_init_counter++ != 0)
        return IK_OK;

    ik_memory_init();
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static uintptr_t
ik_deinit(void)
{
    if (--g_init_counter != 0)
        return 0;

    ik_implement_callbacks(NULL);
    return ik_memory_deinit();
}

/* ------------------------------------------------------------------------- */
struct ik_interface_t IKAPI = {
    ik_init,
    ik_deinit,
    ik_implement_callbacks,
    { IK_BUILD_INFO_STATIC_IMPL },
    { IK_LOG_STATIC_IMPL },
    { IK_QUAT_STATIC_IMPL },
    { IK_SOLVER_STATIC_IMPL },
    { IK_TESTS_STATIC_IMPL },
    { IK_VEC3_STATIC_IMPL },
    {
        &dummy_callbacks,
        { IK_CONSTRAINT_BASE_IMPL},
        { IK_EFFECTOR_BASE_IMPL},
        { IK_NODE_BASE_IMPL },
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
#define X(algorithm) { IK_SOLVER_##algorithm##_IMPL },
        IK_ALGORITHMS
#undef X
    }
};
