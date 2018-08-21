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

void
ik_callbacks_implement(const struct ik_callbacks_t* callbacks);

C_END

#endif /* IK_CALLBACKS_H */
