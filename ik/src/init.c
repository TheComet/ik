#include "cstructures/memory.h"
#include "ik/init.h"
#include "ik/solver.h"
#include <stddef.h>
#include <stdio.h>

static int g_init_counter;

/* ------------------------------------------------------------------------- */
int
ik_init(void)
{
    if (g_init_counter++ != 0)
        return 1;

    if (memory_init() != 0)
        goto memory_init_failed;

    if (ik_solver_init_builtin_interfaces() != 0)
        goto solver_init_interfaces_failed;

    return 0;

    solver_init_interfaces_failed : memory_deinit();
    memory_init_failed            : g_init_counter--;
    return -1;
}

/* ------------------------------------------------------------------------- */
uintptr_t
ik_deinit(void)
{
    if (--g_init_counter != 0)
        return 0;

    ik_solver_deinit_builtin_interfaces();
    return memory_deinit();
}
