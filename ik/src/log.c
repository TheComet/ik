#include "cstructures/memory.h"
#include "cstructures/vector.h"
#include "ik/init.h"
#include "ik/callbacks.h"
#include "ik/log.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

static struct log
{
    struct vector_t message_buffer;
    enum ik_log_severity severity;
    char* prefix;
    int8_t timestamps;
}* g_log;

static int g_init_counter;

/* ------------------------------------------------------------------------- */
void ik_log_set_severity(enum ik_log_severity severity)
{
    g_log->severity = severity;
}

/* ------------------------------------------------------------------------- */
void ik_log_set_timestamps(int enable)
{
    g_log->timestamps = (enable != 0);
}

/* ------------------------------------------------------------------------- */
void ik_log_set_prefix(const char* prefix)
{
    char* buf;
    int len;

    if (g_log->prefix != NULL)
        FREE(g_log->prefix);
    g_log->prefix = NULL;

    if (prefix == NULL)
        return;

    len = strlen(prefix);
    if (len == 0)
        return;

    buf = MALLOC((sizeof(char) + 1) * len);
    if (buf == NULL)
        return;
    strcpy(buf, prefix);
    g_log->prefix = buf;
}

/* ------------------------------------------------------------------------- */
static const char* severities[] = {
#define X(s) #s,
    IK_LOG_SEVERITY_LIST
#undef X
};
void ik_log_printf(enum ik_log_severity severity, const char* fmt, ...)
{
    uintptr_t msg_len;
    char timestamp[12];
    char* buf_ptr;
    va_list va;

    if (g_log == NULL || g_log->severity > severity)
        return;

    if (g_log->timestamps)
    {
        time_t rawtime = time(NULL); /* get system time */
        struct tm* timeinfo = localtime(&rawtime); /* convert to local time */
        strftime(timestamp, 12, "[%X]", timeinfo);
    }

    /* Deterine total length of message */
    va_start(va, fmt);
    msg_len = vsnprintf(NULL, 0, fmt, va);
    va_end(va);
    if (g_log->prefix)
        msg_len += strlen(g_log->prefix) + 3;
    msg_len += 10; /* for tag (DEBUG, INFO, WARNING, ERROR, FATAL)*/
    msg_len += 12; /* for timestamp */
    msg_len += 2;  /* for 2 spaces */

    if (vector_resize(&g_log->message_buffer, (msg_len + 1) * sizeof(char)) < 0)
        return;

    buf_ptr = (char*)g_log->message_buffer.data;
    if (g_log->timestamps)
        buf_ptr += sprintf(buf_ptr, "%s ", timestamp);
    if (g_log->prefix)
        buf_ptr += sprintf(buf_ptr, "[%s] ", g_log->prefix);
    buf_ptr += sprintf(buf_ptr, "%s: ", severities[severity]);
    va_start(va, fmt);
    vsprintf(buf_ptr, fmt, va);
    va_end(va);

    ik_callbacks_notify_log_message((char*)g_log->message_buffer.data);
}

/* ------------------------------------------------------------------------- */
void ik_log_out_of_memory(const char* function)
{
    ik_log_printf(IK_FATAL, "Ran out of memory in function %s", function);
}

/* ------------------------------------------------------------------------- */
int ik_log_init(void)
{
    int result;

    if (g_init_counter++ != 0)
        return IK_OK;

    /* The log depends on the ik library being initialized */
    if ((result = ik_init()) != 0)
        goto ik_init_failed;

    g_log = (struct log*)MALLOC(sizeof *g_log);
    if (g_log == NULL)
    {
        result = IK_ERR_OUT_OF_MEMORY;
        goto alloc_log_failed;
    }

    vector_init(&g_log->message_buffer, sizeof(char));
    g_log->prefix = NULL;
    g_log->timestamps = 1;
#ifdef DEBUG
    ik_log_set_severity(IK_DEBUG);
#else
    ik_log_set_severity(IK_INFO);
#endif

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)
    ik_log_set_prefix(STR(IKAPI));

    return IK_OK;

    alloc_log_failed : ik_deinit();
    ik_init_failed   : return result;
}

/* ------------------------------------------------------------------------- */
void ik_log_deinit(void)
{
    if (--g_init_counter != 0)
        return;

    vector_deinit(&g_log->message_buffer);
    if (g_log->prefix != NULL)
        FREE(g_log->prefix);
    FREE(g_log);
    g_log = NULL;
    ik_deinit();
}
