#ifndef IK_LOG_H
#define IK_LOG_H

#include "ik/config.h"

#define IK_LOG_SEVERITY_LIST \
    X(DEVEL) \
    X(INFO) \
    X(WARNING) \
    X(ERROR) \
    X(FATAL)

C_BEGIN

enum ik_log_severity_e
{
#define X(arg) IK_LOG_##arg,
    IK_LOG_SEVERITY_LIST
#undef X

    IK_LOG_SEVERITY_COUNT
};

#if defined(IK_BUILDING)

IK_PRIVATE_API ikret_t
ik_log_init(void);

IK_PRIVATE_API void
ik_log_deinit(void);

IK_PRIVATE_API void
ik_log_severity(enum ik_log_severity_e severity);

IK_PRIVATE_API void
ik_log_timestamps(int enable);

IK_PRIVATE_API void
ik_log_prefix(const char* prefix);

IK_PRIVATE_API void
ik_log_devel(const char* fmt, ...);
IK_PRIVATE_API void
ik_log_info(const char* fmt, ...);
IK_PRIVATE_API void
ik_log_warning(const char* fmt, ...);
IK_PRIVATE_API void
ik_log_error(const char* fmt, ...);
IK_PRIVATE_API void
ik_log_fatal(const char* fmt, ...);

#endif /* IK_BUILDING */

C_END

#endif /* IK_LOG_H */
