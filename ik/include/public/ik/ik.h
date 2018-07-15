#ifndef IK_LIB_H
#define IK_LIB_H

#include "ik/config.h"
#include "ik/iface/build_info.h"
#include "ik/iface/callback.h"
#include "ik/iface/constraint.h"
#include "ik/iface/effector.h"
#include "ik/iface/log.h"
#include "ik/iface/mat3x3.h"
#include "ik/iface/node.h"
#include "ik/iface/pole.h"
#include "ik/iface/solver.h"
#include "ik/iface/tests.h"
#include "ik/iface/transform.h"

C_BEGIN

struct ik_callback_implementation_t;

struct ik_polymorphic_interface_t
{
    /* Base interface implementations*/
    const struct ik_constraint_interface_t constraint_base;
    const struct ik_effector_interface_t   effector_base;
    const struct ik_node_interface_t       node_base;
    const struct ik_pole_interface_t       pole_base;
    const struct ik_solver_interface_t     solver_base;

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
#define X(algorithm) const struct ik_pole_interface_t pole_##algorithm;
    IK_ALGORITHMS
#undef X
#define X(algorithm) const struct ik_solver_interface_t solver_##algorithm;
    IK_ALGORITHMS
#undef X
};

struct ik_interface_t
{
    ikret_t
    (*init)(void);

    uintptr_t
    (*deinit)(void);

    const struct ik_callback_interface_t    callback;
    const struct ik_build_info_interface_t  info;
    const struct ik_log_interface_t         log;
    const struct ik_mat3x3_interface_t      mat3x3;
    const struct ik_quat_interface_t        quat;
    const struct ik_solver_interface_t      solver;
    const struct ik_tests_interface_t       tests;
    const struct ik_transform_interface_t   transform;
    const struct ik_vec3_interface_t        vec3;

    const struct ik_polymorphic_interface_t base;
};

IK_PUBLIC_API extern const struct ik_interface_t IKAPI;

C_END

#endif /* IK_LIB_H */
