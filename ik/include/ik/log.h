#ifndef IK_LOG_H
#define IK_LOG_H

#include "ik/config.h"

#define IK_LOG_SEVERITY_LIST \
    X(DEBUG)                 \
    X(INFO)                  \
    X(WARN)                  \
    X(ERROR)                 \
    X(FATAL)

enum ik_log_severity
{
#define X(s) IK_##s,
    IK_LOG_SEVERITY_LIST
#undef X
};

C_BEGIN

IK_PUBLIC_API int
ik_log_init(void);

IK_PUBLIC_API void
ik_log_deinit(void);

IK_PUBLIC_API void
ik_log_set_severity(enum ik_log_severity severity);

IK_PUBLIC_API void
ik_log_set_timestamps(int enable);

IK_PUBLIC_API void
ik_log_printf(enum ik_log_severity, const char* fmt, ...);

IK_PUBLIC_API void
ik_log_set_callback(void (*callback)(enum ik_log_severity, const char*));

IK_PUBLIC_API void
ik_log_out_of_memory(const char* func_name);

C_END

#endif /* IK_LOG_H */
