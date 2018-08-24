#ifndef IK_BUILD_INFO_H
#define IK_BUILD_INFO_H

#include "ik/config.h"

C_BEGIN

#if defined(IK_BUILDING)

IK_PRIVATE_API const char*
ik_info_version(void);

IK_PRIVATE_API int
ik_info_build_number(void);

IK_PRIVATE_API const char*
ik_info_host(void);

IK_PRIVATE_API const char*
ik_info_date(void);

IK_PRIVATE_API const char*
ik_info_commit(void);

IK_PRIVATE_API const char*
ik_info_compiler(void);

IK_PRIVATE_API const char*
ik_info_cmake(void);

IK_PRIVATE_API const char*
ik_info_all(void);

#endif /* IK_BUILDING */

C_END

#endif /* IK_BUILD_INFO_H */
