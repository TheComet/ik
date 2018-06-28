#ifndef IK_LIB_H
#define IK_LIB_H

#include "ik/config.h"
#include "ik/solver.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/effector.h"
#include "ik/constraint.h"

C_HEADER_BEGIN

struct ik_callback_interface_t
{
    void
    (*on_log_message)(const char* message);

    void
    (*on_node_destroy)(struct ik_node_t* node);
};

struct ik_internal_interface_t
{
    const struct ik_callback_interface_t* callbacks;

    /* Base interface implementations*/
    const struct ik_constraint_interface_t constraint_base;
    const struct ik_effector_interface_t effector_base;
    const struct ik_node_interface_t node_base;
    const struct ik_solver_interface_t solver_base;

    /* Derived interface implementations */
#define X(algorithm) const struct ik_constraint_interface_t constraint_##algorithm;
    IK_ALGORITHMS
#undef X
#define X(algorithm) const struct ik_effector_interface_t effector_##algorithm;
        IK_ALGORITHMS
#undef X
#define X(algorithm) const struct ik_node_interface_t node_##algorithm;
        IK_ALGORITHMS
#undef X
#define X(algorithm) const struct ik_solver_interface_t solver_##algorithm;
        IK_ALGORITHMS
#undef X
};

struct ik_interface_t
{
    ik_ret
    (*init)(void);

    void
    (*deinit)(void);

    void
    (*implement_callbacks)(const struct ik_callback_interface_t* callbacks);

    const struct ik_log_interface_t log;
    const struct ik_solver_interface_t solver;

    /* "Private" interface, should not be used by clients of the library. */
    struct ik_internal_interface_t internal;
};

IK_PUBLIC_API extern struct ik_interface_t ik;

C_HEADER_END

#endif /* IK_LIB_H */
