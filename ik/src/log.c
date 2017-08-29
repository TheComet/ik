#include "ik/log.h"
#include "ik/memory.h"
#include "ik/vector.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

typedef struct log_t
{
    vector_t listeners; /* list of ik_log_cb_func */
    vector_t message_buffer;
} log_t;

static log_t* g_log = NULL;

static void log_stdout_callback(const char* msg)
{
    puts(msg);
}

/* ------------------------------------------------------------------------- */
void
ik_log_init(enum ik_log_e options)
{
    if (g_log != NULL)
        return;

    g_log = (log_t*)MALLOC(sizeof *g_log);
    if (g_log == NULL)
        return;

    vector_construct(&g_log->listeners, sizeof(ik_log_cb_func));
    vector_construct(&g_log->message_buffer, sizeof(char));

    if (options == IK_LOG_STDOUT)
        ik_log_register_listener(log_stdout_callback);
}

/* ------------------------------------------------------------------------- */
void
ik_log_deinit(void)
{
    if (g_log == NULL)
        return;

    vector_clear_free(&g_log->message_buffer);
    vector_clear_free(&g_log->listeners);
    FREE(g_log);
    g_log = NULL;
}

/* ------------------------------------------------------------------------- */
void
ik_log_register_listener(ik_log_cb_func callback)
{
    if (g_log != NULL)
        vector_push(&g_log->listeners, &callback);
}

/* ------------------------------------------------------------------------- */
void
ik_log_unregister_listener(ik_log_cb_func callback)
{
    if (g_log == NULL)
        return;

    VECTOR_FOR_EACH(&g_log->listeners, ik_log_cb_func, registered_callback)
        if (callback == *registered_callback)
        {
            vector_erase_element(&g_log->listeners, registered_callback);
            return;
        }
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_log_message(const char* fmt, ...)
{
    va_list va;
    uintptr_t msg_len;

    if (g_log == NULL)
        return;

    va_start(va, fmt);
    msg_len = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (vector_resize(&g_log->message_buffer, (msg_len + 1) * sizeof(char)) < 0)
        return;
    va_start(va, fmt);
    vsprintf((char*)g_log->message_buffer.data, fmt, va);
    va_end(va);

    VECTOR_FOR_EACH(&g_log->listeners, ik_log_cb_func, callback)
        (*callback)((char*)g_log->message_buffer.data);
    VECTOR_END_EACH
}
