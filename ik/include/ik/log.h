#ifndef IK_LOG_H
#define IK_LOG_H

#include "ik/config.h"

C_BEGIN

IK_INTERFACE(log_interface)
{
    ikret_t
    (*init)(void);

    void
    (*deinit)(void);

    void
    (*message)(const char* fmt, ...);
};

C_END

#endif /* IK_LOG_H */
