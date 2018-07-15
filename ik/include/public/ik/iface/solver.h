#ifndef IK_SOLVER_H
#define IK_SOLVER_H

#include "ik/config.h"
#include "ik/vector.h"

/*!
 * @brief Only the algorithms listed here are actually enabled.
 *
 * This list is used at multiple locations in the library, specifically in
 * ik_solver_base_create() to fill in the switch/case with information on how
 * to create each solver, but there are also certain expectations on how
 * functions are to be named for each solver.
 *
 * @note Read WHAT_ARE_VTABLES.md in the ik root directory to learn more about
 * the mechanism with which the various solver functions are integrated into
 * the library.
 */
#define IK_ALGORITHMS \
    X(ONE_BONE) \
    X(TWO_BONE) \
    X(FABRIK) \
    X(MSS)

C_BEGIN

enum ik_algorithm_e
{
#define X(solver) IK_##solver,
    IK_ALGORITHMS
#undef X
};

typedef void(*ik_solver_iterate_node_cb_func)(struct ik_node_t*);

struct ik_solver_interface_t;
struct ik_solver_t;
struct ik_node_t;

#define IK_SOLVER_HEAD                                                        \
    const struct ik_solver_interface_t*      v;                               \
                                                                              \
    int32_t                                  max_iterations;                  \
    ikreal_t                                 tolerance;                       \
    uint8_t                                  flags;                           \
                                                                              \
    /* API functions */                                                       \
    const struct ik_constraint_interface_t*  constraint;                      \
    const struct ik_effector_interface_t*    effector;                        \
    const struct ik_node_interface_t*        node;                            \
    const struct ik_pole_interface_t*        pole;                            \
                                                                              \
    struct ik_node_t*                        tree;                            \
                                                                              \
    /* list of effector_t* references (not owned by us) */                    \
    struct vector_t                          effector_nodes_list;             \
    /* list of chain_t objects (allocated in-place, i.e. ik_solver_t owns them) */ \
    struct vector_t                          chain_list;

/*!
 * @brief This is a base for all solvers.
 */
struct ik_solver_t
{
    IK_SOLVER_HEAD
};

enum ik_flags_e
{
    /*!
     * @brief Causes the base node in the tree to be excluded from the list of
     * nodes to solve for. It won't be affected by the solver, but it may still
     * be passed through to the result callback function.
     */
    IK_ENABLE_CONSTRAINTS = 0x01,

    IK_ENABLE_TARGET_ROTATIONS = 0x02,

    IK_ENABLE_JOINT_ROTATIONS = 0x04
};

IK_INTERFACE(solver_interface)
{
    uintptr_t
    (*type_size)(void);

    /*!
     * @brief Allocates a new solver object according to the specified algorithm.
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
     *       there is no need to rebuild data.
     *  + solver->effector_nodes_list
     *       A vector containing pointers to nodes in the tree which have an
     *       effector attached to them. You may not modify this list, but you may
     *       iterate it.
     * @param[in] algorithm The algorithm to use. Currently, only FABRIK is
     * supported.
     */
    struct ik_solver_t*
    (*create)(enum ik_algorithm_e algorithm);

    /*!
     * @brief Destroys the solver and all nodes/effectors that are part of the
     * solver. Any pointers to tree nodes are invalid after this function returns.
     */
    void
    (*destroy)(struct ik_solver_t* solver);

    ikret_t
    (*construct)(struct ik_solver_t* solver);

    void
    (*destruct)(struct ik_solver_t* solver);

    /*!
     * @brief Causes the set tree to be processed into more optimal data structures
     * for solving. Must be called before (*solve)().
     * @note Needs to be called whenever the tree changes in any way. I.e. if you
     * remove nodes or add nodes, or if you remove effectors or add effectors,
     * you must call this again before invoking the solver.
     * @return Returns non-zero if any of the chain trees are invalid for any
     * reason. If this happens, check the log for error messages.
     * @warning If this functions fails, the internal structures are in an
     * undefined state. You cannot solve the tree in this state.
     */
    ikret_t
    (*rebuild)(struct ik_solver_t* solver);

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
     * @note This function gets called by (*rebuild_data)().
     */
    void
    (*update_distances)(struct ik_solver_t* solver);

    /*!
     * @brief Solves the IK problem. The node solutions will be provided via a
     * callback function, which can be registered to the solver by assigning it to
     * solver->apply_result.
     * @return The return value should be 1 if the result converged. 0 if any of
     * the end effectors didn't converge. -1 if there was an error.
     */
    ikret_t
    (*solve)(struct ik_solver_t* solver);

    /*!
     * @brief Sets the tree to solve. The solver takes ownership of the tree, so
     * destroying the solver will destroy all nodes in the tree. You may also
     * pass NULL. If the solver already has a tree, then said tree will be
     * destroyed.
     * @note You will have to call ik_solver_rebuild_data() before being able
     * to solve it.
     * @param[in] solver The solver object.
     * @param[in] tree The root node of a tree you built. Passing NULL will
     * destroy any existing tree the solver owns.
     */
    void
    (*set_tree)(struct ik_solver_t* solver, struct ik_node_t* tree);

    /*!
     * @brief The solver releases any references to a previously set tree and
     * returns the root node of said tree. Any proceeding calls that involve the
     * tree (e.g. solve or rebuild) will have no effect until a new tree is set.
     * @return If the solver has no tree then NULL is returned.
     */
    struct ik_node_t*
    (*unlink_tree)(struct ik_solver_t* solver);

    /*!
     * @brief Iterates all nodes in the internal tree, breadth first, and passes
     * each node to the specified callback function.
     */
    void
    (*iterate_all_nodes)(struct ik_solver_t* solver,
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
    void
    (*iterate_affected_nodes)(struct ik_solver_t* solver,
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
    void
    (*iterate_base_nodes)(struct ik_solver_t* solver,
                          ik_solver_iterate_node_cb_func callback);
};

#define SOLVER_FOR_EACH_EFFECTOR_NODE(solver_var, effector_var) \
    VECTOR_FOR_EACH(&(solver_var)->effector_nodes_list, struct ik_node_t*, solver_var##effector_var) \
    struct ik_node_t* effector_var = *solver_var##effector_var; {

#define SOLVER_FOR_EACH_CHAIN(solver_var, base_chain_var) \
    VECTOR_FOR_EACH(&(solver_var)->chain_list, struct chain_t, base_chain_var) {

#define SOLVER_END_EACH VECTOR_END_EACH }

C_END

#endif /* IK_SOLVER_H */
