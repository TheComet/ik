#pragma once

#include "ik/config.h"
#include "ik/refcount.h"

C_BEGIN

struct ik_solver;
struct ik_algorithm;
struct ik_subtree;
struct ik_bone;

#define IK_SOLVER_HEAD                                                        \
    IK_REFCOUNTED_HEAD                                                        \
                                                                              \
    struct ik_solver_interface impl;                                          \
    struct ik_algorithm* algorithm;                                           \
    struct ik_bone* root_bone;

typedef void(*ik_bone_visit_func)(struct ik_bone*, void* param);

struct ik_solver_interface
{
    /*! Unique string that identifies this solver. This is matched against the
     * string in ik_algorithm to instantiate a solver when building */
    const char* name;

    /*! Size of the solver struct, e.g. sizeof(struct my_solver) */
    uintptr_t size;

    /*! Called When the solver is allocated. The base fields will be initialized,
     * so don't use memset() to set your fields. */
    int (*init)(struct ik_solver*, const struct ik_subtree*);

    /*! Called before the solver is freed. */
    void (*deinit)(struct ik_solver*);

    /*! Called when it's time to solve. Returns the number of iterations that
     * were required to reach the solution */
    int (*solve)(struct ik_solver*);

    /*! Call the specified callback function for every node that the solver
     * affects. The order must be from base node to leaf node(s), depth first. */
    void (*visit_bones)(const struct ik_solver*, ik_bone_visit_func visit, void* param);
    void (*visit_effectors)(const struct ik_solver*, ik_bone_visit_func visit, void* param);
};

IK_PUBLIC_API int
ik_solver_register(const struct ik_solver_interface* interface);

IK_PUBLIC_API int
ik_solver_unregister(const struct ik_solver_interface* interface);

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
IK_PUBLIC_API struct ik_solver*
ik_solver_build(struct ik_bone* root);

IK_PUBLIC_API int
ik_solver_solve(struct ik_solver* solver);

IK_PUBLIC_API void
ik_solver_visit_bones(const struct ik_solver* solver, ik_bone_visit_func visit, void* param);

IK_PUBLIC_API void
ik_solver_visit_effectors(const struct ik_solver* solver, ik_bone_visit_func visit, void* param);

#if defined(IK_BUILDING)

IK_PRIVATE_API int
ik_solver_init_builtin_interfaces(void);

IK_PRIVATE_API void
ik_solver_deinit_builtin_interfaces(void);

IK_PRIVATE_API struct ik_solver*
ik_solver_alloc(const struct ik_solver_interface* impl,
               struct ik_algorithm* algorithm,
               struct ik_bone* root_bone);

IK_PRIVATE_API void
ik_solver_free(struct ik_solver* solver);

#endif

C_END
