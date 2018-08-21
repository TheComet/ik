#ifndef IK_RETCODES_H
#define IK_RETCODES_H

/* Don't use C_BEGIN because config.h includes retcodes.h */
#ifdef __cplusplus
extern "C" {
#endif

typedef enum ikret_t
{
    IK_RESULT_CONVERGED = 1,
    IK_OK = 0,
    IK_RAN_OUT_OF_MEMORY = -1,
    IK_ALREADY_HAS_ATTACHMENT = -2,
    IK_HASH_NOT_FOUND = -3,
    IK_HASH_EXISTS = -4,
    IK_HASH_RESERVED = -5,
    IK_VECTOR_HAS_DIFFERENT_ELEMENT_SIZE = -6,
    IK_SOLVER_HAS_NO_TREE = -7,
    IK_UNIT_TESTS_FAILED = -8,
    IK_BUILT_WITHOUT_TESTS = -9,
    IK_WRONG_FUNCTION_FOR_CUSTOM_CONSTRAINT = -10
} ikret_t;

#ifdef __cplusplus
}
#endif

#endif
