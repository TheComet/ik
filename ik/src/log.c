#include "ik/log.h"
#include "ik/memory.h"
#include "ik/ordered_vector.h"

static struct log_t
{
    struct ordered_vector_t listeners; /* list of ik_log_cb_func */
}* g_log = NULL;

/* ------------------------------------------------------------------------- */
void
ik_log_init(void)
{
    if(g_log != NULL)
        return;

    g_log = (struct log_t*)MALLOC(sizeof *g_log);
    if(g_log == NULL)
        return;

    ordered_vector_construct(&g_log->listeners, sizeof(ik_log_cb_func));
}

/* ------------------------------------------------------------------------- */
void
ik_log_deinit(void)
{
    if(g_log == NULL)
        return;

    ordered_vector_clear_free(&g_log->listeners);
    FREE(g_log);
}

/* ------------------------------------------------------------------------- */
void
ik_log_register_listener(ik_log_cb_func callback)
{
    if(g_log != NULL)
        ordered_vector_push(&g_log->listeners, &callback);
}

/* ------------------------------------------------------------------------- */
void
ik_log_unregister_listener(ik_log_cb_func callback)
{
    if(g_log == NULL)
        return;

    ORDERED_VECTOR_FOR_EACH(&g_log->listeners, ik_log_cb_func, registered_callback)
        if(callback == *registered_callback)
        {
            ordered_vector_erase_element(&g_log->listeners, registered_callback);
            return;
        }
    ORDERED_VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_log_message(const char* message)
{
    if(g_log == NULL)
        return;

    ORDERED_VECTOR_FOR_EACH(&g_log->listeners, ik_log_cb_func, callback)
        (*callback)(message);
    ORDERED_VECTOR_END_EACH
}
