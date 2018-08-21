#ifndef IK_LOG_H
#define IK_LOG_H

#include "ik/config.h"

C_BEGIN

enum ik_log_severity_e
{
    IK_DEBUG = 0,
    IK_INFO,
    IK_WARNING,
    IK_ERROR,
    IK_FATAL
};

IK_PUBLIC_API ikret_t
ik_log_init(void);

IK_PUBLIC_API void
ik_log_deinit(void);

IK_PUBLIC_API void
ik_log_severity(enum ik_log_severity_e severity);

IK_PUBLIC_API void
ik_log_timestamps(int enable);

IK_PUBLIC_API void
ik_log_prefix(const char* prefix);

IK_PUBLIC_API void
ik_log_debug(const char* fmt, ...);
IK_PUBLIC_API void
ik_log_info(const char* fmt, ...);
IK_PUBLIC_API void
ik_log_warning(const char* fmt, ...);
IK_PUBLIC_API void
ik_log_error(const char* fmt, ...);
IK_PUBLIC_API void
ik_log_fatal(const char* fmt, ...);

C_END

#endif /* IK_LOG_H */
