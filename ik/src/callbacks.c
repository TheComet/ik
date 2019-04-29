#include "ik/callbacks.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
static void
log_stdout_callback(const char* msg)
{
    puts(msg);
}

struct ik_callbacks_t ik_callbacks;

/* ------------------------------------------------------------------------- */
void
ik_callbacks_init(void)
{
    memset(&ik_callbacks, 0, sizeof ik_callbacks);
    ik_callbacks.on_log_message = log_stdout_callback;
}

/* ------------------------------------------------------------------------- */
void
ik_callbacks_notify_log_message(const char* message)
{
    if (ik_callbacks.on_log_message)
        ik_callbacks.on_log_message(message);
}

/* ------------------------------------------------------------------------- */
void
ik_callbacks_notify_node_free(struct ik_node_t* node)
{
    if (ik_callbacks.on_node_free)
        ik_callbacks.on_node_free(node);
}
