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

ikret_t
ik_log_init(void);

void
ik_log_deinit(void);

void
ik_log_severity(enum ik_log_severity_e severity);

void
ik_log_timestamps(int enable);

void
ik_log_prefix(const char* prefix);

void
ik_log_debug(const char* fmt, ...);
void
ik_log_info(const char* fmt, ...);
void
ik_log_warning(const char* fmt, ...);
void
ik_log_error(const char* fmt, ...);
void
ik_log_fatal(const char* fmt, ...);

#endif /* IK_LOG_H */
