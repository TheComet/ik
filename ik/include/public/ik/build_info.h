#ifndef IK_BUILD_INFO_H
#define IK_BUILD_INFO_H

#include "ik/config.h"

C_BEGIN

const char*
ik_build_info_version(void);

int
ik_build_info_build_number(void);

const char*
ik_build_info_host(void);

const char*
ik_build_info_date(void);

const char*
ik_build_info_commit(void);

const char*
ik_build_info_compiler(void);

const char*
ik_build_info_cmake(void);

const char*
ik_build_info_all(void);

C_END

#endif /* IK_BUILD_INFO_H */
