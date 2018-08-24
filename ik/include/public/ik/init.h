#ifndef IK_INIT_H
#define IK_INIT_H

#include "ik/config.h"

C_BEGIN

#if defined(IK_BUILDING)

IK_PRIVATE_API ikret_t
ik_init(void);

IK_PRIVATE_API uintptr_t
ik_deinit(void);

#endif /* IK_BUILDING */

C_END

#endif /* IK_INIT_H */
