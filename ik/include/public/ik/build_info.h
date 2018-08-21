#ifndef IK_BUILD_INFO_H
#define IK_BUILD_INFO_H

#include "ik/config.h"

C_BEGIN

IK_PUBLIC_API const char*
ik_build_info_version(void);

IK_PUBLIC_API int
ik_build_info_build_number(void);

IK_PUBLIC_API const char*
ik_build_info_host(void);

IK_PUBLIC_API const char*
ik_build_info_date(void);

IK_PUBLIC_API const char*
ik_build_info_commit(void);

IK_PUBLIC_API const char*
ik_build_info_compiler(void);

IK_PUBLIC_API const char*
ik_build_info_cmake(void);

IK_PUBLIC_API const char*
ik_build_info_all(void);

C_END

#endif /* IK_BUILD_INFO_H */
