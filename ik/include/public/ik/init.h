#ifndef IK_INIT_H
#define IK_INIT_H

#include "ik/config.h"

C_BEGIN

IK_PUBLIC_API ikret_t
ik_init(void);

IK_PUBLIC_API uintptr_t
ik_deinit(void);

C_END

#endif /* IK_INIT_H */
