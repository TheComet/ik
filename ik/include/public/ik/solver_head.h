#ifndef IK_TYPES_H
#define IK_TYPES_H

#include "ik/config.h"
#include "ik/attachment.h"

C_BEGIN

/*!
 * @brief This is a base for all solvers.
 */
#define IK_SOLVER_HEAD                                                        \
    IK_ATTACHMENT_HEAD                                                        \
                                                                              \
    /* Derived interface */                                                   \
    ikret_t (*construct)(struct ik_solver_t* solver);                         \
    void    (*destruct)(struct ik_solver_t* solver);                          \
    ikret_t (*prepare)(struct ik_solver_t* solver);                           \
    ikret_t (*solve)(struct ik_solver_t* solver);                             \
                                                                              \
    /* Used to push/pop transformations as the trees are iterated. This is    \
     * allocated in prepare() if alloca() is not supported, or if the stack   \
     * is larger than IK_MAX_STACK_ALLOC. */                                  \
    uint8_t* stack_buffer;                                                    \
                                                                              \
    /* List of ik_chain_t objects. Holds all chains that connect to an        \
     * effector for fast updates of target information. */                    \
    struct vector_t effector_chains;                                          \
                                                                              \
    /* list of ik_ntf_t objects  */                                           \
    struct vector_t      ntf_list;                                            \
                                                                              \
    /* Solver parameters shared among all solvers */                          \
    ikreal_t             tolerance;                                           \
    uint16_t             max_iterations;                                      \
    uint16_t             features;

C_END

#endif /* IK_TYPES_H */
