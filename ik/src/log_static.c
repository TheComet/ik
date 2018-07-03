#include "ik/log_static.h"
#include "ik/memory.h"
#include "ik/vector.h"
#include "ik/ik.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

typedef struct log_t
{
    struct vector_t message_buffer;
    uint8_t severity;
} log_t;

static log_t* g_log = NULL;
static int g_init_counter = 0;

/* ------------------------------------------------------------------------- */
ikret_t
ik_log_static_init(void)
{
    ikret_t result;

    if (g_init_counter++ != 0)
        return IK_OK;

    /* The log depends on the ik library being initialized */
    if ((result = IKAPI.init()) != IK_OK)
        goto ik_init_failed;

    g_log = (log_t*)MALLOC(sizeof *g_log);
    if (g_log == NULL)
    {
        result = IK_RAN_OUT_OF_MEMORY;
        goto alloc_log_failed;
    }

    vector_construct(&g_log->message_buffer, sizeof(char));
#ifdef DEBUG
    ik_log_static_set_severity(IK_DEBUG);
#else
    ik_log_static_set_severity(IK_INFO);
#endif

    return IK_OK;

    alloc_log_failed : IKAPI.deinit();
    ik_init_failed   : return result;
}

/* ------------------------------------------------------------------------- */
void
ik_log_static_deinit(void)
{
    if (--g_init_counter != 0)
        return;

    vector_clear_free(&g_log->message_buffer);
    FREE(g_log);
    g_log = NULL;
    IKAPI.deinit();
}

/* ------------------------------------------------------------------------- */
void
ik_log_static_set_severity(enum ik_log_severity_e severity)
{
    if (g_log == NULL)
        return;

    /* Need to map to a strictly increasing number for easier comparison later. 
     * IK_DEBUG starts at 0 */
    switch (severity)
    {
        case IK_DEBUG   : g_log->severity = 0; break;
        case IK_INFO    : g_log->severity = 1; break;
        case IK_WARNING : g_log->severity = 2; break;
        case IK_ERROR   : g_log->severity = 3; break;
        case IK_FATAL   : g_log->severity = 4; break;
    }
}

/* ------------------------------------------------------------------------- */
void
ik_log_static_message(const char* fmt, ...)
{
    va_list va;
    uintptr_t msg_len;

    if (g_log == NULL)
        return;

    /* Discard the message if its severity level is beneath the configured one */
    switch (fmt[0])
    {
        case IK_DEBUG   : if (g_log->severity > 0) return; break;
        case IK_INFO    : if (g_log->severity > 1) return; break;
        case IK_WARNING : if (g_log->severity > 2) return; break;
        case IK_ERROR   : if (g_log->severity > 3) return; break;
        case IK_FATAL   : if (g_log->severity > 4) return; break;
        default: break; /* assume nothing */
    }

    va_start(va, fmt);
    msg_len = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (vector_resize(&g_log->message_buffer, (msg_len + 1) * sizeof(char)) < 0)
        return;
    va_start(va, fmt);
    vsprintf((char*)g_log->message_buffer.data, fmt, va);
    va_end(va);

    if (IKAPI.internal.callbacks->on_log_message != NULL)
        IKAPI.internal.callbacks->on_log_message((char*)g_log->message_buffer.data);
}
