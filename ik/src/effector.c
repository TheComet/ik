#include "ik/effector.h"
#include "ik/memory.h"
#include "ik/node.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
struct effector_t*
effector_create(void)
{
    struct effector_t* effector = (struct effector_t*)MALLOC(sizeof *effector);
    if(effector == NULL)
        return NULL;

    effector_construct(effector);
    return effector;
}

/* ------------------------------------------------------------------------- */
void
effector_construct(struct effector_t* effector)
{
    memset(effector, 0, sizeof *effector);
}

/* ------------------------------------------------------------------------- */
void
effector_destroy(struct effector_t* effector)
{
    FREE(effector);
}

/* ------------------------------------------------------------------------- */
void
effector_attach(struct effector_t* effector, struct node_t* node)
{
    if(node->effector != NULL)
        effector_destroy(node->effector);

    node->effector = effector;
}
