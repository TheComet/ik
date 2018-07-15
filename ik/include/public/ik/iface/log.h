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

IK_INTERFACE(log_interface)
{
    ikret_t
    (*init)(void);

    void
    (*deinit)(void);

    void
    (*severity)(enum ik_log_severity_e severity);

    void
    (*timestamps)(int enable);

    void
    (*prefix)(const char* prefix);

    void
    (*debug)(const char* fmt, ...);
    void
    (*info)(const char* fmt, ...);
    void
    (*warning)(const char* fmt, ...);
    void
    (*error)(const char* fmt, ...);
    void
    (*fatal)(const char* fmt, ...);
};

C_END

#endif /* IK_LOG_H */
