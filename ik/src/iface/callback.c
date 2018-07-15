#include "ik/impl/callback.h"
#include <stdio.h>

/* ------------------------------------------------------------------------- */
static void
log_stdout_callback(const char* msg)
{
    puts(msg);
}

/* ------------------------------------------------------------------------- */
static const struct ik_callback_interface_t dummy_callback = {
    log_stdout_callback,
    NULL,
    NULL
};
/* ------------------------------------------------------------------------- */
static const struct ik_callback_interface_t* g_callback = &dummy_callback;

/* ------------------------------------------------------------------------- */
void
ik_callback_on_log_message(const char* message)
{
    if (g_callback->on_log_message)
        g_callback->on_log_message(message);
}

/* ------------------------------------------------------------------------- */
void
ik_callback_on_node_destroy(struct ik_node_t* node)
{
    if (g_callback->on_node_destroy)
        g_callback->on_node_destroy(node);
}

/* ------------------------------------------------------------------------- */
void
ik_callback_implement(const struct ik_callback_interface_t* callback)
{
    if (callback)
        g_callback = callback;
    else
        g_callback = &dummy_callback;
}
