#include "ik/node_FABRIK.h"
#include "ik/memory.h"
#include "ik/ik.h"
#include <stddef.h>

/* ------------------------------------------------------------------------- */
struct ik_node_t*
ik_node_FABRIK_create(uint32_t guid)
{
    struct ik_node_FABRIK_t* node = MALLOC(sizeof *node);
    if (node == NULL)
    {
        ik.log.message("fFailed to allocate node: Ran out of memory");
        return NULL;
    }

    ik.internal.node_FABRIK.construct((struct ik_node_t*)node, guid);

    return (struct ik_node_t*)node;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_node_FABRIK_construct(struct ik_node_t* node_base, uint32_t guid)
{
    node_base->v = &IKAPI.internal.node_FABRIK;
    return IK_OK;
}
