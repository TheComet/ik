#include "ik/effector_base.h"
#include "ik/ik.h"
#include "ik/memory.h"
#include "ik/vec3.h"
#include "ik/quat.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
struct ik_effector_t*
ik_effector_base_create(void)
{
    struct ik_effector_t* effector = MALLOC(sizeof *effector);
    if (effector == NULL)
        return NULL;

    memset(effector, 0, sizeof *effector);
    vec3_set_zero(effector->target_position.f);
    quat_set_identity(effector->target_rotation.f);
    effector->weight = 1.0;
    effector->rotation_weight = 1.0;
    effector->rotation_decay = 0.25;
    effector->v = &IK.internal.effector_base;

    return effector;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_base_destroy(struct ik_effector_t* effector)
{
    ik_effector_base_detach(effector);
    FREE(effector);
}

/* ------------------------------------------------------------------------- */
int
ik_effector_base_attach(struct ik_effector_t* effector, struct ik_node_t* node)
{
    if (node->effector != NULL)
    {
        IK.log.message(
            "Warning! You are trying to attach an effector to a node that "
            "already has an effector attached to it. The new effector will not "
            "be attached!"
        );
        return -1;
    }

    /* effector may be attached to another node */
    ik_effector_base_detach(effector);

    node->effector = effector;
    effector->node = node;

    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_base_detach(struct ik_effector_t* effector)
{
    if (effector->node == NULL)
        return;

    effector->node->effector = NULL;
    effector->node = NULL;
}
