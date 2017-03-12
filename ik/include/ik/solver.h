#ifndef IK_SOLVER_H
#define IK_SOLVER_H

#include "ik/gen/config.h"
#include "ik/pstdint.h"
#include "ik/ordered_vector.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_HEADER_BEGIN

struct ik_effector_t;
struct ik_node_t;
struct ik_solver_t;

typedef void (*ik_solver_destroy_func)(struct ik_solver_t*);
typedef int (*ik_solver_rebuild_data_func)(struct ik_solver_t*);
typedef int (*ik_solver_solve_func)(struct ik_solver_t*);
typedef void (*ik_solver_reset_func)(struct ik_solver_t*);

typedef void (*ik_solver_apply_constraint_cb_func)(struct ik_node_t*);
typedef void (*ik_solver_apply_result_cb_func)(struct ik_node_t*, vec3_t, quat_t);

enum algorithm_e
{
    SOLVER_FABRIK,
    SOLVER_JACOBIAN_INVERSE,
    SOLVER_JACOBIAN_TRANSPOSE
};

enum build_mode_e
{
    SOLVER_INCLUDE_ROOT = 0,
    SOLVER_EXCLUDE_ROOT
};

/*!
 * @brief This is a base struct for all solvers.
 */
#define SOLVER_DATA_HEAD                                             \
    ik_solver_apply_constraint_cb_func apply_constraint;             \
    ik_solver_apply_result_cb_func     apply_result;                 \
                                                                     \
    int32_t                            max_iterations;               \
    float                              tolerance;                    \
    enum build_mode_e                  build_mode;                   \
                                                                     \
    /* Derived structure callbacks */                                \
    ik_solver_destroy_func             destroy;                      \
    ik_solver_rebuild_data_func        rebuild_data;                 \
    ik_solver_solve_func               solve;                        \
    ik_solver_reset_func               reset;                        \
                                                                     \
    struct ordered_vector_t            effector_nodes_list;          \
                                                                     \
    struct ik_node_t*                  tree;
struct ik_solver_t
{
    SOLVER_DATA_HEAD
};

IK_PUBLIC_API struct ik_solver_t*
ik_solver_create(enum algorithm_e algorithm);

IK_PUBLIC_API void
ik_solver_destroy(struct ik_solver_t* solver);

IK_PUBLIC_API void
ik_solver_set_tree(struct ik_solver_t* solver, struct ik_node_t* root);

IK_PUBLIC_API void
ik_solver_destroy_tree(struct ik_solver_t* solver);

IK_PUBLIC_API int
ik_solver_rebuild_data(struct ik_solver_t* solver);

IK_PUBLIC_API int
ik_solver_solve(struct ik_solver_t* solver);

IK_PUBLIC_API void
ik_solver_reset(struct ik_solver_t* solver);

C_HEADER_END

#endif /* IK_SOLVER_H */
