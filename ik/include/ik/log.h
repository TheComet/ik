#ifndef IK_LOG_H
#define IK_LOG_H

#include "ik/config.h"

C_HEADER_BEGIN

typedef void (*ik_log_cb_func)(const char*);

IK_PUBLIC_API void
ik_log_init(void);

IK_PUBLIC_API void
ik_log_deinit(void);

IK_PUBLIC_API void
ik_log_register_listener(ik_log_cb_func callback);

IK_PUBLIC_API void
ik_log_unregister_listener(ik_log_cb_func callback);

IK_PUBLIC_API void
ik_log_message(const char* message);

C_HEADER_END

#endif /* IK_LOG_H */
