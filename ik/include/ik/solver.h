#ifndef IK_SOLVER_H
#define IK_SOLVER_H

#include "ik/config.h"
#include "ik/pstdint.h"
#include "ik/ordered_vector.h"

C_HEADER_BEGIN

struct solver_t;
struct node_t;

typedef void (*ik_solver_destroy_func)(struct solver_t*);
typedef int (*ik_solver_solve_func)(struct solver_t*);
typedef void (*ik_solver_constraint_cb_func)(struct solver_t*);

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
struct solver_data_t
{
    ik_solver_destroy_func destroy;
    ik_solver_solve_func solve;

    int32_t max_iterations;
    float tolerance;

    struct node_t* tree;
    struct ordered_vector_t effector_nodes_list;
};
#define SOLVER_DATA_HEAD             \
    union                            \
    {                                \
        struct solver_data_t solver; \
    } base;
struct solver_t
{
    SOLVER_DATA_HEAD
};

struct solver_t*
ik_solver_create(enum algorithm_e algorithm);

void
ik_solver_destroy(struct solver_t* solver);

struct node_t*
ik_solver_create_tree_root(struct solver_t* solver, uint32_t guid);

int
ik_solver_solve(struct solver_t* solver);

C_HEADER_END

#endif /* IK_SOLVER_H */
