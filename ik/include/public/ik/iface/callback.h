#ifndef IK_CALLBACKS_H
#define IK_CALLBACKS_H

#include "ik/config.h"

C_BEGIN

struct ik_node_t;

IK_INTERFACE(callback_interface)
{
    void
    (*on_log_message)(const char* message);

    void
    (*on_node_destroy)(struct ik_node_t* node);

    void
    (*implement)(const IK_INTERFACE(callback_interface)* callbacks);
};

C_END

#endif /* IK_CALLBACKS_H */
