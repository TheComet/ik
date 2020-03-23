#ifndef IK_CALLBACKS_H
#define IK_CALLBACKS_H

#include "ik/config.h"

C_BEGIN

struct ik_node_t;

struct ik_callbacks_t
{
    void
    (*on_log_message)(const char* message);

    void
    (*on_node_free)(struct ik_node_t* node);
};

IK_PUBLIC_API extern struct ik_callbacks_t ik_callbacks;

IK_PRIVATE_API void
ik_callbacks_init(void);

IK_PRIVATE_API void
ik_callbacks_notify_log_message(const char* message);

IK_PRIVATE_API void
ik_callbacks_notify_node_free(struct ik_node_t* node);

C_END

#endif /* IK_CALLBACKS_H */
