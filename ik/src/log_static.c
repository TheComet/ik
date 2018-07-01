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
} log_t;

static log_t* g_log = NULL;
static int g_init_counter = 0;

/* ------------------------------------------------------------------------- */
ikret_t
ik_log_static_init(void)
{
    if (g_init_counter++ != 0)
        return IK_OK;

    /* The log depends on the ik library being initialized */
    if (ik.init() != IK_OK)
        goto ik_init_failed;

    g_log = (log_t*)MALLOC(sizeof *g_log);
    if (g_log == NULL)
        goto alloc_log_failed;

    vector_construct(&g_log->message_buffer, sizeof(char));

    return IK_OK;

    alloc_log_failed : ik.deinit();
    ik_init_failed   : return IK_RAN_OUT_OF_MEMORY;
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
    ik.deinit();
}

/* ------------------------------------------------------------------------- */
void
ik_log_static_message(const char* fmt, ...)
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

    if (ik.internal.callbacks->on_log_message != NULL)
        ik.internal.callbacks->on_log_message((char*)g_log->message_buffer.data);
}
