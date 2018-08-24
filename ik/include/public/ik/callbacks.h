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
    (*on_node_destroy)(struct ik_node_t* node);
};

#if defined(IK_BUILDING)

IK_PRIVATE_API void
ik_callbacks_implement(const struct ik_callbacks_t* callbacks);

IK_PRIVATE_API void
ik_callbacks_notify_log_message(const char* message);

IK_PRIVATE_API void
ik_callbacks_notify_node_destroy(struct ik_node_t* node);

#endif /* IK_BUILDING */

C_END

#endif /* IK_CALLBACKS_H */
