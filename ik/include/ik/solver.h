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
typedef void (*ik_solver_recalculate_segment_lengths_func)(struct ik_solver_t*);
typedef int (*ik_solver_solve_func)(struct ik_solver_t*);
typedef void (*ik_solver_reset_func)(struct ik_solver_t*);

typedef void (*ik_solver_apply_constraint_cb_func)(struct ik_node_t*);
typedef void (*ik_solver_apply_result_cb_func)(struct ik_node_t*);

enum solver_algorithm_e
{
    SOLVER_FABRIK,
    SOLVER_JACOBIAN_INVERSE,
    SOLVER_JACOBIAN_TRANSPOSE
};

enum solver_flags_e
{
    SOLVER_EXCLUDE_ROOT                = 0x01,
    SOLVER_CALCULATE_FINAL_ANGLES      = 0x02,
    SOLVER_CALCULATE_CONSTRAINT_ANGLES = 0x04,
    SOLVER_SKIP_RESET                  = 0x08,
    SOLVER_SKIP_APPLY                  = 0x10
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
    uint8_t                            flags;                        \
                                                                     \
    /* Derived structure callbacks */                                \
    ik_solver_destroy_func             destroy;                      \
    ik_solver_rebuild_data_func        rebuild_data;                 \
    ik_solver_recalculate_segment_lengths_func recalculate_segment_lengths; \
    ik_solver_solve_func               solve;                        \
    ik_solver_reset_func               reset;                        \
                                                                     \
    struct ordered_vector_t            effector_nodes_list;          \
    struct ik_node_t*                  tree;
struct ik_solver_t
{
    SOLVER_DATA_HEAD
};

/*!
 * @brief Allocates a new solver object according to the specified algorithm.
 * @param[in] algorithm The algorithm to use. Currently, only FABRIK is
 * supported.
 */
IK_PUBLIC_API struct ik_solver_t*
ik_solver_create(enum solver_algorithm_e algorithm);

/*!
 * @brief Destroys the solver and all nodes/effectors that are part of the
 * solver. Any pointers to tree nodes are invalid after this function returns.
 */
IK_PUBLIC_API void
ik_solver_destroy(struct ik_solver_t* solver);

/*!
 * @brief Sets the tree to solve. The solver takes ownership of the tree, so
 * destroying the solver will destroy all nodes in the tree. Note that you will
 * have to call ik_solver_rebuild_data() before being able to solve it. If the
 * solver already has a tree, then said tree will be destroyed.
 */
IK_PUBLIC_API void
ik_solver_set_tree(struct ik_solver_t* solver, struct ik_node_t* root);

/*!
 * @brief The solver releases any references to a previously set tree and
 * returns the root node of said tree. Any proceeding calls that involve the
 * tree (e.g. solve or rebuild) will have no effect until a new tree is set.
 * @return If the solver has no tree then NULL is returned.
 */
IK_PUBLIC_API struct ik_node_t*
ik_solver_unlink_tree(struct ik_solver_t* solver);

/*!
 * @brief The solver releases any references to a previously set tree and
 * destroys it.
 */
IK_PUBLIC_API void
ik_solver_destroy_tree(struct ik_solver_t* solver);

/*!
 * @brief Causes the set tree to be processed into more optimal data structures
 * for solving. Must be called before ik_solver_solve().
 * @note Needs to be called whenever the tree changes in any way. I.e. if you
 * remove nodes or add nodes, or if you remove effectors or add effectors,
 * you must call this again before calling the solver.
 */
IK_PUBLIC_API int
ik_solver_rebuild_data(struct ik_solver_t* solver);

/*!
 * @brief Unusual, but if you have a tree with translational motions such that
 * the distances between nodes changes (perhaps a slider?), you can call this
 * to re-calculate the segment lengths after assigning new positions to the
 * nodes.
 * @note This function gets called by ik_solver_rebuild_data().
 */
IK_PUBLIC_API void
ik_solver_recalculate_segment_lengths(struct ik_solver_t* solver);

/*!
 * @brief Solves the IK problem. The node solutions will be provided via a
 * callback function, which can be registered to the solver by assigning it to
 * solver->apply_result.
 */
IK_PUBLIC_API int
ik_solver_solve(struct ik_solver_t* solver);

/*!
 * @brief Iterates all nodes in the internal tree, breadth first, and calls the
 * solver->apply_result callback function for every node.
 *
 * Typically, you would call this after solving the tree to apply the results
 * back to your own scene graph. This function could also be used to reset your
 * own scene graph to its initial state by reading the node->position and
 * node->rotation properties.
 */
IK_PUBLIC_API void
ik_solver_iterate_tree(struct ik_solver_t* solver);

C_HEADER_END

#endif /* IK_SOLVER_H */
