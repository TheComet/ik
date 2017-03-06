#ifndef IK_LOG_H
#define IK_LOG_H

#include "ik/config.h"
#include "ik/ordered_vector.h"

C_HEADER_BEGIN

typedef void (*ik_log_cb_func)(const char*);

struct log_t
{
    struct ordered_vector_t listeners; /* list of ik_log_cb_func */
};

IK_PUBLIC_API struct log_t*
ik_log_create(void);

IK_PUBLIC_API void
ik_log_construct(struct log_t* log);

IK_PUBLIC_API void
ik_log_destruct(struct log_t* log);

IK_PUBLIC_API void
ik_log_destroy(struct log_t* log);

IK_PUBLIC_API void
ik_log_register_listener(struct log_t* log, ik_log_cb_func callback);

IK_PUBLIC_API void
ik_log_unregister_listener(struct log_t* log, ik_log_cb_func callback);

IK_PUBLIC_API void
ik_log_message(struct log_t* log, const char* message);

C_HEADER_END

#endif /* IK_LOG_H */
