#ifndef IK_TYPES_H
#define IK_TYPES_H

#include "ik/config.h"
#include "ik/attachment.h"

C_BEGIN

/*!
 * @brief This is a base for all algorithms.
 */
#define IK_ALGORITHM_HEAD                                                     \
    IK_ATTACHMENT_HEAD                                                        \
                                                                              \
    /* Derived interface */                                                   \
    ikret_t (*construct)(struct ik_algorithm_t* algorithm);                   \
    void    (*destruct)(struct ik_algorithm_t* algorithm);                    \
    ikret_t (*prepare)(struct ik_algorithm_t* algorithm);                     \
    ikret_t (*solve)(struct ik_algorithm_t* algorithm);                       \
                                                                              \
    /* Used to push/pop transformations as the trees are iterated. This is    \
     * allocated in prepare() if alloca() is not supported, or if the stack   \
     * is larger than IK_MAX_STACK_ALLOC. */                                  \
    uint8_t* stack_buffer;                                                    \
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
