#include "cstructures/memory.h"
#include "ik/ik.h"
#include "ik/callbacks.h"
#include <stddef.h>

static int g_init_counter;

/* ------------------------------------------------------------------------- */
ikret
ik_init(void)
{
    if (g_init_counter++ != 0)
        return IK_OK;

    memory_init();
    ik_callbacks_init();

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
uintptr_t
ik_deinit(void)
{
    if (--g_init_counter != 0)
        return 0;

    return memory_deinit();
}
