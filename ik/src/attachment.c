#include "ik/attachment.h"
#include <stddef.h>

/* ------------------------------------------------------------------------- */
struct ik_attachment*
ik_attachment_alloc(uintptr_t obj_size, ik_deinit_func deinit)
{
    struct ik_attachment* att = (struct ik_attachment*)
        ik_refcounted_alloc(obj_size, deinit);
    if (att == NULL)
        return NULL;

    att->node = NULL;
    return att;
}
