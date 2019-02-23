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
    ikret_t (*prepare)(struct ik_solver_t* solver);                           \
    ikret_t (*solve)(struct ik_solver_t* solver);                             \
    uint8_t* stack_buffer;                                                    \
                                                                              \
    /* list of ik_ntf_t objects  */                                           \
    struct vector_t      ntf_list;                                            \
                                                                              \
    ikreal_t             tolerance;                                           \
    uint16_t             max_iterations;                                      \
    uint16_t             features;

C_END

#endif /* IK_TYPES_H */
