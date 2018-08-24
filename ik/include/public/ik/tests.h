#ifndef IK_TESTS_H
#define IK_TESTS_H

#include "ik/config.h"

C_BEGIN

#if defined(IK_BUILDING)

IK_PRIVATE_API ikret_t
ik_tests_run(int* argc, char** argv);

#endif /* IK_BUILDING */

C_END

#endif /* IK_TESTS_H */
