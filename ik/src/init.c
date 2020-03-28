#include "cstructures/memory.h"
#include "ik/init.h"
#include "ik/solver.h"
#include "ik/callbacks.h"
#include <stddef.h>

static int g_init_counter;

/* ------------------------------------------------------------------------- */
int
ik_init(void)
{
    if (g_init_counter++ != 0)
        return 0;

    if (memory_init() != 0)
        goto memory_init_failed;

    if (ik_solver_init_interfaces() != 0)
        goto solver_init_interfaces_failed;

    ik_callbacks_init();

    return 0;

    solver_init_interfaces_failed : memory_deinit();
    memory_init_failed            : return -1;
}

/* ------------------------------------------------------------------------- */
uintptr_t
ik_deinit(void)
{
    if (--g_init_counter != 0)
        return 0;

    ik_solver_deinit_interfaces();
    return memory_deinit();
}
