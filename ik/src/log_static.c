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
    char* prefix;
    int8_t timestamps;
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
    g_log->prefix = NULL;
    g_log->timestamps = 1;
#ifdef DEBUG
    ik_log_static_set_severity(IK_DEBUG);
#else
    ik_log_static_set_severity(IK_INFO);
#endif
    ik_log_static_prefix("ik");

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
    if (g_log->prefix != NULL)
        FREE(g_log->prefix);
    FREE(g_log);
    g_log = NULL;
    IKAPI.deinit();
}

/* ------------------------------------------------------------------------- */
void
ik_log_static_severity(enum ik_log_severity_e severity)
{
    /* Need to map to a strictly increasing number for easier comparison later. */
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
ik_log_static_timestamps(int enable)
{
    g_log->timestamps = (enable != 0);
}

/* ------------------------------------------------------------------------- */
void
ik_log_static_prefix(const char* prefix)
{
    char* buf;
    int len = strlen(prefix);

    if (g_log->prefix != NULL)
        FREE(g_log->prefix);
    g_log->prefix = NULL;

    if (len == 0)
        return;

    buf = MALLOC((sizeof(char) + 1) * len);
    if (buf == NULL)
        return;
    strcpy(buf, prefix);
    g_log->prefix = buf;
}

/* ------------------------------------------------------------------------- */
static void
log_message(enum ik_log_severity_e severity, const char* fmt, va_list vargs)
{
    uintptr_t msg_len;
    time_t rawtime;
    struct tm* timeinfo;
    char timestamp[12];
    const char* tag;
    const char* buf_ptr;
    va_list vargs_copy;

    rawtime = time(NULL); /* get system time */
    timeinfo = localtime(&rawtime); /* convert to local time */
    strftime(timestamp, 12, "[%X] ", timeinfo);

    switch (severity)
    {
        case IK_DEBUG:   tag = "[DEBUG]";   break;
        case IK_INFO:    tag = "[INFO]";    break;
        case IK_WARNING: tag = "[WARNING]"; break;
        case IK_ERROR:   tag = "[ERROR]";   break;
        case IK_FATAL:   tag = "[FATAL]";   break;
    }

    /* Deterine total length of message */
    va_copy(vargs_copy, vargs);
    msg_len = vsnprintf(NULL, 0, fmt, vargs_copy);
    va_end(vargs_copy);
    if (g_log->prefix)
        msg_len += strlen(g_log->prefix) + 3;
    msg_len += 10; /* for tag */
    msg_len += 12; /* for timestamp */
    msg_len += 2;  /* for 2 spaces */

    if (vector_resize(&g_log->message_buffer, (msg_len + 1) * sizeof(char)) < 0)
        return;

    buf_ptr = (char*)g_log->message_buffer.data;
    if (g_log->prefix)
        buf_ptr += sprintf(buf_ptr, "[%s] ", g_log->prefix);
    buf_ptr += sprintf(g_log->message_buffer.data, "[%s] [%s] ", tag, timestamp);
    vsprintf(buf_ptr, fmt, vargs);

    if (ik_callback->on_log_message != NULL)
        ik_callback->on_log_message((char*)g_log->message_buffer.data);
}
