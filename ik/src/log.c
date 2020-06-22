#include "cstructures/memory.h"
#include "cstructures/vector.h"
#include "ik/init.h"
#include "ik/log.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

static struct log
{
    struct cs_vector message_buffer;
    void (*write_message)(enum ik_log_severity severity, const char* msg);
    enum ik_log_severity severity;
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
void ik_log_printf(enum ik_log_severity severity, const char* fmt, ...)
{
    uintptr_t msg_len;
    char* buf_ptr;
    va_list va;

    if (g_log == NULL || g_log->severity > severity)
        return;

    /* Deterine total length of message */
    va_start(va, fmt);
    msg_len = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (vector_resize(&g_log->message_buffer, (msg_len + 1) * sizeof(char)) < 0)
        return;

    buf_ptr = (char*)g_log->message_buffer.data;

    va_start(va, fmt);
    vsprintf(buf_ptr, fmt, va);
    va_end(va);

    g_log->write_message(severity, (const char*)g_log->message_buffer.data);
}

/* ------------------------------------------------------------------------- */
void
ik_log_set_callback(void (*callback)(enum ik_log_severity, const char* msg))
{
    g_log->write_message = callback;
}

/* ------------------------------------------------------------------------- */
void ik_log_out_of_memory(const char* function)
{
    ik_log_printf(IK_FATAL, "Ran out of memory in function %s", function);
}

/* ------------------------------------------------------------------------- */
static const char* severities[] = {
#define X(s) #s,
    IK_LOG_SEVERITY_LIST
#undef X
};
static void
write_stderr(enum ik_log_severity severity, const char* msg)
{
    char timestamp[9];

    if (g_log->timestamps)
    {
        time_t rawtime = time(NULL); /* get system time */
        struct tm* timeinfo = localtime(&rawtime); /* convert to local time */
        strftime(timestamp, 9, "%H:%M:%S", timeinfo);  /* HH:MM:SS + null = 9 bytes */
        fprintf(stderr, "[%s] ", timestamp);
    }

    fprintf(stderr, "%s: %s\n", severities[severity], msg);
}

/* ------------------------------------------------------------------------- */
int ik_log_init(void)
{
    if (g_init_counter++ != 0)
        return 1;

    /* The log depends on the ik library being initialized */
    if (ik_init() < 0)
        goto ik_init_failed;

    g_log = (struct log*)MALLOC(sizeof *g_log);
    if (g_log == NULL)
        goto alloc_log_failed;

    vector_init(&g_log->message_buffer, sizeof(char));
    g_log->write_message = write_stderr;
    g_log->timestamps = 1;
#ifdef DEBUG
    ik_log_set_severity(IK_DEBUG);
#else
    ik_log_set_severity(IK_INFO);
#endif

    return 0;

    alloc_log_failed : ik_deinit();
    ik_init_failed   : g_init_counter--;
    return -1;
}

/* ------------------------------------------------------------------------- */
void ik_log_deinit(void)
{
    if (--g_init_counter != 0)
        return;

    vector_deinit(&g_log->message_buffer);
    FREE(g_log);
    g_log = NULL;
    ik_deinit();
}
