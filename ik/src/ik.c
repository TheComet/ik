#include "ik/ik.h"
#include "ik/constraint_base.h"
#include "ik/effector_base.h"
#include "ik/log_static.h"
#include "ik/memory.h"
#include "ik/node_base.h"
#include "ik/solver_static.h"
#include "ik/solver_base.h"
#include "ik/solver_ONE_BONE.h"
#include "ik/solver_TWO_BONE.h"
#include "ik/solver_FABRIK.h"
#include "ik/solver_MSS.h"
#include "ik/tests_static.h"
#include <stddef.h>
#include <stdio.h>

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
        ik.internal.callbacks = callbacks;
    else
        ik.internal.callbacks = &dummy_callbacks;
}

/* ------------------------------------------------------------------------- */
static ikret_t
ik_init(void)
{
    ik_memory_init();
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
static uintptr_t
ik_deinit(void)
{
    ik_implement_callbacks(NULL);
    return ik_memory_deinit();
}

/* ------------------------------------------------------------------------- */
struct ik_interface_t ik = {
    ik_init,
    ik_deinit,
    ik_implement_callbacks,
    { IK_LOG_STATIC_IMPL },
    { IK_SOLVER_STATIC_IMPL },
    { IK_TESTS_STATIC_IMPL },
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
