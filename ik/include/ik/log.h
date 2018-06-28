#ifndef IK_LOG_H
#define IK_LOG_H

#include "ik/config.h"

C_HEADER_BEGIN

IK_INTERFACE(log_interface)
{
    ik_ret
    (*init)(void);

    void
    (*deinit)(void);

    void
    (*message)(const char* fmt, ...);
};

C_HEADER_END

#endif /* IK_LOG_H */
