#ifndef IK_SOLVER_H
#define IK_SOLVER_H

#include "ik/config.h"

C_BEGIN

struct ik_solver;

typedef void(*ik_solver_callback_func)(void* user_data,
                                       const ikreal position[3],
                                       const ikreal rotation[4]);

typedef ikret (*ik_solver_init_func)   (struct ik_solver*);
typedef void  (*ik_solver_deinit_func) (struct ik_solver*);
typedef ikret (*ik_solver_prepare_func)(struct ik_solver*);
typedef ikret (*ik_solver_solve_func)  (struct ik_solver*);

/*!
 * @brief This is a base for all solvers.
 */
#define IK_SOLVER_HEAD                                                        \
    /* Derived interface */                                                   \
    ik_solver_init_func          init;                                        \
    ik_solver_deinit_func        deinit;                                      \
    ik_solver_prepare_func       prepare;                                     \
    ik_solver_solve_func         solve;                                       \
                                                                              \
    struct ik_algorithm_t*       algorithm;

struct ik_solver
{
    IK_SOLVER_HEAD
};

/*!
 * @brief Allocates a new solver object according to the specified solver.
 *
 * Once the solver is created, you can configure the solver to enable/disable
 * various features depending on your needs.
 *
 * The following attributes can be changed at any point.
 *  + solver->max_iterations
 *       Specifies the maximum number of iterations. The more iterations, the
 *       more exact the result will be. The default value for the FABRIK solver
 *       is 20, but you can get away with values as low as 5.
 *  + solver->tolerance
 *       This value can be changed at any point. Specifies the acceptable
 *       distance each effector needs to be to its target position. The solver
 *       will stop iterating if the effectors are within this distance. The
 *       default value is 1e-3. Recommended values are 100th of your world
 *       unit.
 *  + solver->flags
 *       Changes the behaviour of the solver. See the enum solver_flags_e for
 *       more information.
 *
 * The following attributes can be accessed (read from) but should not be
 * modified.
 *  + solver->tree
 *       The tree to be solved. You may modify the nodes in the tree.
 *       @note If you add/remove nodes or if you add/remove effectors, you
 *       must call ik_solver_rebuild_data() so the internal solver structures
 *       are updated. Failing to do so may cause segfaults. If you're just
 *       updating positions/rotations or any of the other public data then
 *       there is no need to rebuild daIK_ALGORITHM_LISTta.
 *  + solver->effector_nodes_list
 *       A vector containing pointers to nodes in the tree which have an
 *       effector attached to them. You may not modify this list, but you may
 *       iterate it.
 * @param[in] solver The solver to use. Currently, only FABRIK is
 * supported.
 */
IK_PRIVATE_API ikret
ik_solver_create(struct ik_solver** solver,
                 struct ik_algorithm_t* algorithm,
                 struct ik_node_data_t* node_data,
                 uint32_t subbase_idx, uint32_t chain_begin_idx, uint32_t chain_end_idx);

/*!
 * @brief Destroys the solver and all nodes/effectors that are part of the
 * solver. Any pointers to tree nodes are invalid after this function returns.
 */
IK_PRIVATE_API void
ik_solver_free(struct ik_solver* solver);

/*!
 * @brief Computes the distances between the nodes and stores them in
 * node->dist_to_parent. The positions used for this computation are those of
 * the active pose (node->position). For this reason, make sure that you've
 * correctly initialized the active pose before calling this function.
 *
 * The segment lengths are typically computed once during initialisation and
 * then never again. Of course, there are exceptions, such as when your tree
 * has translational motions. In this case, you will have to recalculate the
 * segment lengths every time node positions change.
 *
 * @note This function gets called by ik_solver_prepare().
 */
IK_PRIVATE_API void
ik_solver_update_translations(struct ik_solver* solver);

IK_PRIVATE_API void
ik_solver_iterate_nodes(const struct ik_solver* solver, ik_solver_callback_func cb);

C_END

#endif /* IK_SOLVER_H */
