#include "ik/init.h"
#include "ik/memory.h"
#include "ik/impl/callback.h"
#include <stddef.h>

static int g_init_counter = 0;

/* ------------------------------------------------------------------------- */
ikret_t
ik_init(void)
{
    if (g_init_counter++ != 0)
        return IK_OK;

    ik_memory_init();
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
uintptr_t
ik_deinit(void)
{
    if (--g_init_counter != 0)
        return 0;

    ik_callback_implement(NULL);
    return ik_memory_deinit();
}
