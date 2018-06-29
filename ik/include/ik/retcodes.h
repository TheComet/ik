#ifndef IK_RETCODES_H
#define IK_RETCODES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ikret_t
{
    IK_RESULT_CONVERGED = 1,
    IK_OK = 0,
    IK_RAN_OUT_OF_MEMORY = -1,
    IK_ALREADY_HAS_ATTACHMENT = -2,
    IK_ALREADY_INITIALIZED = -3,
    IK_HASH_NOT_FOUND = -4,
    IK_VECTOR_HAS_DIFFERENT_ELEMENT_SIZE = -5,
    IK_SOLVER_HAS_NO_TREE = -6,
    IK_UNIT_TESTS_FAILED = -7
} ikret_t;

#ifdef __cplusplus
}
#endif

#endif
