#include "ik/log.h"
#include "ik/memory.h"

/* ------------------------------------------------------------------------- */
struct log_t*
ik_log_create(void)
{
    struct log_t* log = (struct log_t*)MALLOC(sizeof *log);
    if(log == NULL)
        return NULL;

    ik_log_construct(log);
    return log;
}

/* ------------------------------------------------------------------------- */
void
ik_log_construct(struct log_t* log)
{
    ordered_vector_construct(&log->listeners, sizeof(ik_log_cb_func));
}

/* ------------------------------------------------------------------------- */
void
ik_log_destruct(struct log_t* log)
{
    ordered_vector_clear_free(&log->listeners);
}

/* ------------------------------------------------------------------------- */
void
ik_log_destroy(struct log_t* log)
{
    ik_log_destruct(log);
    FREE(log);
}

/* ------------------------------------------------------------------------- */
void
ik_log_register_listener(struct log_t* log, ik_log_cb_func callback)
{
    ordered_vector_push(&log->listeners, &callback);
}

/* ------------------------------------------------------------------------- */
void
ik_log_unregister_listener(struct log_t* log, ik_log_cb_func callback)
{
    ORDERED_VECTOR_FOR_EACH(&log->listeners, ik_log_cb_func, registered_callback)
        if(callback == *registered_callback)
        {
            ordered_vector_erase_element(&log->listeners, registered_callback);
            return;
        }
    ORDERED_VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_log_message(struct log_t* log, const char* message)
{

    ORDERED_VECTOR_FOR_EACH(&log->listeners, ik_log_cb_func, callback)
        (*callback)(message);
    ORDERED_VECTOR_END_EACH
}
