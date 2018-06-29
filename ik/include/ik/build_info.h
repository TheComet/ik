#ifndef IK_BUILD_INFO_H
#define IK_BUILD_INFO_H

#include "ik/config.h"

C_BEGIN

IK_PUBLIC_API const char*
ik_version(void);

IK_PUBLIC_API int
ik_build_number(void);

IK_PUBLIC_API const char*
ik_build_host(void);

IK_PUBLIC_API const char*
ik_build_time(void);

IK_PUBLIC_API const char*
ik_commit_info(void);

IK_PUBLIC_API const char*
ik_compiler_info(void);

IK_PUBLIC_API const char*
ik_cmake_configuration(void);

IK_PUBLIC_API const char*
ik_build_info(void);

C_END

#endif /* IK_BUILD_INFO_H */
