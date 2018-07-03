#ifndef IK_TESTS_H
#define IK_TESTS_H

#include "ik/config.h"

C_BEGIN

IK_INTERFACE(tests_interface)
{
    ikret_t
    (*run)(void);
};

C_END

#endif /* IK_TESTS_H */
