#include "ik/python/init.h"
#include "ik/ik.h"

static int g_refcount = 0;

/* ------------------------------------------------------------------------- */
int
init_iklib_refcounted(void)
{
    if (g_refcount == 0)
    {
        if (ik.init() != IK_OK)
            goto ik_init_failed;
        if (ik.log.init() != IK_OK)
            goto ik_log_init_failed;
    }

    g_refcount++;
    return 0;

    ik_log_init_failed : ik.deinit();
    ik_init_failed     : return -1;
}

/* ------------------------------------------------------------------------- */
void
deinit_iklib_refcounted(void)
{
    if (--g_refcount != 0)
        return;

    ik.log.deinit();
    ik.deinit();
}
