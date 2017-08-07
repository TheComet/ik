#ifndef IK_SOLVER_H
#define IK_SOLVER_H

#include "ik/config.h"
#include "ik/chain_tree.h"
#include "ik/ordered_vector.h"
#include "ik/quat.h"
#include "ik/vec3.h"

C_HEADER_BEGIN

typedef void (*ik_solver_destruct_func)(ik_solver_t*);
typedef int (*ik_solver_post_chain_build_func)(ik_solver_t*);
typedef int (*ik_solver_solve_func)(ik_solver_t*);

typedef void (*ik_solver_iterate_node_cb_func)(ik_node_t*);

typedef enum solver_algorithm_e
{
    SOLVER_ONE_BONE,
    SOLVER_TWO_BONE,
    SOLVER_FABRIK,
    SOLVER_MSD
    /* TODO Not implemented
    SOLVER_JACOBIAN_INVERSE,
    SOLVER_JACOBIAN_TRANSPOSE */
} solver_algorithm_e;

typedef enum solver_flags_e
{
    /*!
     * @brief Causes the base node in the tree to be excluded from the list of
     * nodes to solve for. It won't be affected by the solver, but it may still
     * be passed through to the result callback function.
     */
    SOLVER_EXCLUDE_BASE                   = 0x01,

    SOLVER_ENABLE_CONSTRAINTS             = 0x02,

    SOLVER_CALCULATE_TARGET_ROTATIONS     = 0x04,

    SOLVER_CALCULATE_JOINT_ROTATIONS      = 0x08
} solver_flags_e;

/*!
 * @brief This is a base for all solvers.
 */
#define SOLVER_DATA_HEAD                                              \
    int32_t                             max_iterations;               \
    float                               tolerance;                    \
    uint8_t                             flags;                        \
                                                                      \
    /* Derived structure callbacks */                                 \
    ik_solver_destruct_func             destruct;                     \
    ik_solver_post_chain_build_func     post_chain_build;             \
    ik_solver_solve_func                solve;                        \
                                                                      \
    ordered_vector_t                    effector_nodes_list;          \
    ik_node_t*                          tree;                         \
    /* list of ik_chain_tree_t objects (allocated in-place) */        \
    chain_tree_t                        chain_tree;

struct ik_solver_t
{
    SOLVER_DATA_HEAD
};

/*!
 * @brief Allocates a new solver object according to the specified algorithm.
 *
 * Once the solver is created, you can configure the solver to enable/disable
 * various features depending on your needs.
 *
 * The following attributes can be changed at any point.
 *  + solver->apply_result
 *       This is the main mechanism with which to obtain the solved data.
 *       Assign a callback function here and it will be called for every node
 *       in the tree when a new target position/rotation has been calculated.
 *       You can use the node->user_data attribute to store external node
 *       specific data, which can be accessed again the in callback function.
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
 *       there is no need to rebuild data.
 *  + solver->effector_nodes_list
 *       A vector containing pointers to nodes in the tree which have an
 *       effector attached to them. You may not modify this list, but you may
 *       iterate it.
 * @param[in] algorithm The algorithm to use. Currently, only FABRIK is
 * supported.
 */
IK_PUBLIC_API ik_solver_t*
ik_solver_create(solver_algorithm_e algorithm);

/*!
 * @brief Destroys the solver and all nodes/effectors that are part of the
 * solver. Any pointers to tree nodes are invalid after this function returns.
 */
IK_PUBLIC_API void
ik_solver_destroy(ik_solver_t* solver);

/*!
 * @brief Sets the tree to solve. The solver takes ownership of the tree, so
 * destroying the solver will destroy all nodes in the tree. Note that you will
 * have to call ik_solver_rebuild_data() before being able to solve it. If the
 * solver already has a tree, then said tree will be destroyed.
 */
IK_PUBLIC_API void
ik_solver_set_tree(ik_solver_t* solver, ik_node_t* base);

/*!
 * @brief The solver releases any references to a previously set tree and
 * returns the base node of said tree. Any proceeding calls that involve the
 * tree (e.g. solve or rebuild) will have no effect until a new tree is set.
 * @return If the solver has no tree then NULL is returned.
 */
IK_PUBLIC_API ik_node_t*
ik_solver_unlink_tree(ik_solver_t* solver);

/*!
 * @brief The solver releases any references to a previously set tree and
 * destroys it.
 */
IK_PUBLIC_API void
ik_solver_destroy_tree(ik_solver_t* solver);

/*!
 * @brief Causes the set tree to be processed into more optimal data structures
 * for solving. Must be called before ik_solver_solve().
 * @note Needs to be called whenever the tree changes in any way. I.e. if you
 * remove nodes or add nodes, or if you remove effectors or add effectors,
 * you must call this again before calling the solver.
 * @return Returns non-zero if any of the chain trees are invalid for any
 * reason. If this happens, check the log for error messages.
 * @warning If this functions fails, the internal structures are in an
 * undefined state. You cannot solve the tree in this state.
 */
IK_PUBLIC_API int
ik_solver_rebuild_chain_trees(ik_solver_t* solver);

/*!
 * @brief Computes the distances between the nodes and stores them in
 * node->segment_length. The positions used for this computation are those of
 * the active pose (node->position). For this reason, make sure that you've
 * correctly initialised the active pose before calling this function.
 *
 * The segment lengths are typically computed once during initialisation and
 * then never again. Of course, there are exceptions, such as when your tree
 * has translational motions. In this case, you will have to recalculate the
 * segment lengths every time node positions change.
 *
 * @note This function gets called by ik_solver_rebuild_data().
 */
IK_PUBLIC_API void
ik_solver_recalculate_segment_lengths(ik_solver_t* solver);

/*!
 * @brief Solves the IK problem. The node solutions will be provided via a
 * callback function, which can be registered to the solver by assigning it to
 * solver->apply_result.
 * @return The return value should be 1 if the result converged. 0 if any of
 * the end effectors didn't converge. -1 if there was an error.
 */
IK_PUBLIC_API int
ik_solver_solve(ik_solver_t* solver);

IK_PUBLIC_API void
ik_solver_calculate_joint_rotations(ik_solver_t* solver);

/*!
 * @brief Iterates all nodes in the internal tree, breadth first, and passes
 * each node to the specified callback function.
 */
IK_PUBLIC_API void
ik_solver_iterate_tree(ik_solver_t* solver,
                       ik_solver_iterate_node_cb_func callback);

/*!
 * @brief Iterates just the nodes that are being affected by the solver,
 * *EXCLUDING* the island base nodes.
 *
 * @note Requires a rebuild before this data is valid.
 *
 * The reason for excluding island base nodes is because their positions and
 * rotations are typically set separately from the rest of the tree (see
 * ik_solver_iterate_base_nodes).
 */
IK_PUBLIC_API void
ik_solver_iterate_chain_tree(ik_solver_t* solver,
                             ik_solver_iterate_node_cb_func callback);

/*!
 * @brief Iterates all nodes that mark the beginning of a subtree.
 *
 * In a lot of cases, the scene graph of a user library is only partially
 * replicated for solving. Because of this, the base nodes of all of the chains
 * in the tree won't be affected by a potential parent node if it is moved or
 * rotated, since the ik library doesn't know about any parent nodes -- It
 * believes the base nodes ARE the parent-most nodes in the tree.
 *
 * To overcome this, you can iterate all of these base nodes and copy the
 * *global* (world) position/rotation into each base node position/rotation.
 * This will correctly position/rotate the solver's chains.
 */
IK_PUBLIC_API void
ik_solver_iterate_base_nodes(ik_solver_t* solver,
                             ik_solver_iterate_node_cb_func callback);

/*!
 * @brief Sets the solved positions and rotations equal to the original
 * positions and rotations for every node in the tree.
 */
IK_PUBLIC_API void
ik_solver_reset_to_original_pose(ik_solver_t* solver);

C_HEADER_END

#endif /* IK_SOLVER_H */
