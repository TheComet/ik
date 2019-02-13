#ifndef IK_TYPES_H
#define IK_TYPES_H

#include "ik/config.h"

C_BEGIN

/*!
 * @brief This is a base for all solvers.
 */
#define SOLVER_HEAD                                                           \
    /* Derived interface */                                                   \
    ikret_t (*construct)(struct ik_solver_t* solver);                         \
    void    (*destruct)(struct ik_solver_t* solver);                          \
    ikret_t (*rebuild)(struct ik_solver_t* solver);                           \
    ikret_t (*solve)(struct ik_solver_t* solver);                             \
                                                                              \
    /* list of effector_t* references (not owned by us) */                    \
    struct vector_t      effector_nodes_list;                                 \
    /* list of chain_t objects (allocated in-place, i.e. ik_solver_t owns them) */\
    struct vector_t      chain_list;                                          \
                                                                              \
    ikreal_t             tolerance;                                           \
    uint16_t             max_iterations;                                      \
    uint8_t              features;

C_END

#endif /* IK_TYPES_H */
