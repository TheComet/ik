#ifndef IK_TYPES_H
#define IK_TYPES_H

#include "ik/config.h"
#include "ik/attachment.h"

C_BEGIN

struct ik_algorithm_t;
typedef ikret_t (*ik_algorithm_construct_func)(struct ik_algorithm_t*);
typedef void    (*ik_algorithm_destruct_func) (struct ik_algorithm_t*);
typedef ikret_t (*ik_algorithm_prepare_func)  (struct ik_algorithm_t*);
typedef ikret_t (*ik_algorithm_solve_func)    (struct ik_algorithm_t*);

/*!
 * @brief This is a base for all algorithms.
 */
#define IK_ALGORITHM_HEAD                                                     \
    IK_ATTACHMENT_HEAD                                                        \
                                                                              \
    /* Derived interface */                                                   \
    ik_algorithm_construct_func    construct;                                 \
    ik_algorithm_destruct_func     destruct;                                  \
    ik_algorithm_prepare_func      prepare;                                   \
    ik_algorithm_solve_func        solve;                                     \
                                                                              \
    /* Weak ref to tree structure this algo is responsible for solving  */    \
    struct ik_ntf_t* ntf;                                                     \
                                                                              \
    /* Solver parameters shared among all algorithms */                       \
    ikreal_t             tolerance;                                           \
    uint16_t             max_iterations;                                      \
    uint16_t             features;

C_END

#endif /* IK_TYPES_H */
