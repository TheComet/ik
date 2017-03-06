#ifndef IK_SOLVER_H
#define IK_SOLVER_H

#include "ik/config.h"
#include "ik/pstdint.h"
#include "ik/ordered_vector.h"
#include "ik/vector3.h"
#include "ik/quaternion.h"

C_HEADER_BEGIN

struct effector_t;
struct log_t;
struct node_t;
struct solver_t;

typedef void (*ik_solver_destroy_func)(struct solver_t*);
typedef int (*ik_solver_solve_func)(struct solver_t*);
typedef int (*ik_solver_rebuild_data_func)(struct solver_t*);

typedef void (*ik_solver_apply_constraint_cb_func)(struct node_t*);
typedef void (*ik_solver_apply_result_cb_func)(struct node_t*, struct vector3_t, struct quaternion_t);

enum algorithm_e
{
    ALGORITHM_FABRIK,
    ALGORITHM_JACOBIAN_INVERSE,
    ALGORITHM_JACOBIAN_TRANSPOSE
};

/*!
 * @brief This is a base struct for all solvers.
 * @note Custom polymorphism, using information from here:
 * http://www.deleveld.dds.nl/inherit.htm
 */
struct solver_t
{
    ik_solver_apply_constraint_cb_func apply_constraint;
    ik_solver_apply_result_cb_func     apply_result;

    int32_t                            max_iterations;
    float                              tolerance;

    struct log_t*                      log;

    struct
    {
        /* Derived structure callbacks */
        ik_solver_destroy_func             destroy;
        ik_solver_solve_func               solve;
        ik_solver_rebuild_data_func        rebuild_data;

        struct ordered_vector_t            effector_nodes_list;

        struct node_t* tree;
    } private_;

};
#define SOLVER_DATA_HEAD             \
    union                            \
    {                                \
        struct solver_t solver;      \
    } base;

IK_PUBLIC_API struct solver_t*
ik_solver_create(enum algorithm_e algorithm);

IK_PUBLIC_API void
ik_solver_destroy(struct solver_t* solver);

IK_PUBLIC_API void
ik_solver_set_tree(struct solver_t* solver, struct node_t* root);

IK_PUBLIC_API int
ik_solver_rebuild_data(struct solver_t* solver);

IK_PUBLIC_API int
ik_solver_solve(struct solver_t* solver);

C_HEADER_END

#endif /* IK_SOLVER_H */
